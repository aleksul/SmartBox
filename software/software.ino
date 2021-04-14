/*
  SmartBox
  Copyright (C) 2021  Aleksul, aleksandrsulimov@bk.ru

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <EEPROM.h>
#include <Wiegand.h>  // https://github.com/paulo-raca/YetAnotherArduinoWiegandLibrary
#define PIN_D0 2
#define PIN_D1 3
Wiegand wg;

#define BUZZER_PIN 9
#define BUTTON_PIN 8
#define LED_PIN LED_BUILTIN
#define EEPROM_LENGTH 1024
#define RELE_1 5
#define RELE_DELAY 5000
#define BUTTON_DELAY 100
#define BUTTON_LONG_PRESS_TIME 10000
#define BUZZER_PIP_TIME (BUTTON_LONG_PRESS_TIME/10)
#define BUTTON_RESET_TIME 30000

uint8_t button_flag = 0;
uint8_t mastercard_flag = 0;
uint8_t delete_flag = 0;

uint8_t massiv[3] = {0, 0, 0};
bool card_read_flag = false;

unsigned long reset_button_timer = 0;

void setup() { 
  delay(100);  // wait for power to stabilize
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELE_1, OUTPUT);
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  wg.onReceive(receivedData, "");
  wg.begin(26, true);
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);

  // sends the initial pin state to the Wiegand library
  pinStateChanged();

  // it will open the locks if the memory is empty
  if (eeprom_last_adress()==0) {
    digitalWrite(RELE_1, HIGH);
  }
}

// updates pin states inside the library
void pinStateChanged() {
  wg.setPin0State(digitalRead(PIN_D0));
  wg.setPin1State(digitalRead(PIN_D1));
}

void loop() {
  manage_button();
  if (card_read_flag) {
    card_read_flag = false;
    cardReadHandler(massiv);
  }
}

/*----------------------Functions--------------------------*/ 


void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  card_read_flag = true;
  for (int i=0; i < 3; i++)
    massiv[i] = data[i];
}


void cardReadHandler(uint8_t* data) {
  // open the locks, if the button was not pressed
  if (eeprom_find_same(data) && button_flag == 0 && mastercard_flag == 0 && delete_flag == 0) {
    digitalWrite(RELE_1, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(RELE_DELAY);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELE_1, LOW);  
  }

  // button long press and master-key == delete everything  
  if (delete_flag == 1) {
    if (is_mastercard(data)) {
      clear_eeprom();
    }
    delete_flag = 0;
    mastercard_flag = 0;
    button_flag = 0;
    digitalWrite(LED_PIN, LOW);  
  }
  
  // write new key
  if (mastercard_flag == 1) {
    mastercard_flag = 0;
    button_flag = 0;
    new_card_write(data);
    digitalWrite(LED_PIN, LOW);
  } 

  // is it master-key?       
  if (button_flag == 1) {
    if (is_mastercard(data)) {
      mastercard_flag = 1;
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    } else if (eeprom_3bytes_free(0)) {  // what if there is no keys?
      new_card_write(data);
      // than the locks were open from the start, we need to close them
      digitalWrite(RELE_1, LOW);
      digitalWrite(LED_PIN, LOW);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
    button_flag = 0;  // we need to reset the button anyway      
  }
}


// finds last empty adress
uint16_t eeprom_last_adress (void) {
  uint16_t last_adress = 0;
  while (!eeprom_3bytes_free(last_adress))
    last_adress += 3;
  return last_adress; 
}


// checks for 3 bytes to be free
boolean eeprom_3bytes_free (uint16_t start_number) { 
  if (start_number > (EEPROM_LENGTH-3))
    return false;
  for (uint8_t i = 0; i < 3; i++) 
    if (EEPROM.read(start_number+i) != 255)
      return false;
  return true;
}


// cheks if the key is master-key
boolean is_mastercard(uint8_t* data){
  if (data[0] == EEPROM.read(0) && data[1] == EEPROM.read(1) && data[2] == EEPROM.read(2)) {
    return true;
  }
  return false;
}


// writes 3 bytes in EEPROM
void eeprom_write (uint16_t start_number, uint8_t* data) { 
  EEPROM.update(start_number+0, data[0]);
  EEPROM.update(start_number+1, data[1]);
  EEPROM.update(start_number+2, data[2]);
}


// tries to find same key in EEPROM
boolean eeprom_find_same(uint8_t* data) { 
  uint16_t start_number = 0;
  while(!eeprom_3bytes_free(start_number)) {
    if ((data[0] != EEPROM.read(start_number+0)) || (data[1] != EEPROM.read(start_number+1)) || \
        (data[2] != EEPROM.read(start_number+2))) {
      start_number = start_number+3;  
    } else {
      return true;
    }
  }
  return false;  
}


// writes key in EEPROM
void new_card_write (uint8_t* data) { 
  if (!eeprom_find_same(data)) {
    uint16_t start_number = eeprom_last_adress();
    eeprom_write(start_number, data);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
  }
}


// clears all EEPROM
void clear_eeprom (void) {
  for (int i = 0; i < 1024; i++)
    EEPROM.update(i, 255);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);  
  digitalWrite(LED_PIN, LOW);
  for (int i = 0; i < 5; i++) {
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  }
}


// resets button state after a while
void reset_new_card_mode (void) {
  if (reset_button_timer == 0) {
    reset_button_timer = millis();
  }
  if (millis() - reset_button_timer >= BUTTON_RESET_TIME) {
    button_flag = 0;
    delete_flag = 0;
    reset_button_timer = 0;
    digitalWrite(LED_PIN, LOW);
  }
}


void manage_button(void){
  unsigned long button_timer = 0;
  unsigned long start_point = millis();
  uint8_t beeps_counter = 0;
  if (delete_flag == 1)
    return;
  while(digitalRead(BUTTON_PIN) == HIGH) {
     button_timer = millis() - start_point;
     // beeps every second
     if (button_timer / BUZZER_PIP_TIME > beeps_counter) {
        beeps_counter++;
        if (beeps_counter >= 10) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(1000);
          digitalWrite(BUZZER_PIN, LOW);
          delete_flag = 1;
          break;
        }
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
     }
     if (button_flag == 0) {
       delay(100);  // protection against phantom clicks
       if (digitalRead(BUTTON_PIN) == HIGH) {
         button_flag = 1;
         digitalWrite(LED_PIN, HIGH);
       }
     }
  }
  if (button_flag == 1 || delete_flag == 1) {
    reset_new_card_mode();
  }
}
