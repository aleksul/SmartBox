#include <Wiegand.h>
#include <EEPROM.h>
WIEGAND wg;

#define BUTTON_PIN 5
#define LED_PIN 10
#define EEPROM_LENGTH 1024
#define RELE_1 8
#define RELE_2 9
#define RELE_DELAY 1000
#define BUTTON_DELAY 100
#define BUTTON_LONG_PRESS_TIME 10000

uint8_t button_flag = 0;
uint16_t button_long_press = 0;

void setup() { 
	wg.begin();
  pinMode(BUTTON_PIN, INPUT); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELE_1, OUTPUT);
  pinMode(RELE_2, OUTPUT);
}


void loop() {
  
  if(digitalRead(BUTTON_PIN) == HIGH){
    button_flag = 1;
    digitalWrite(LED_PIN, HIGH);
  }
  
	if(wg.available())
	{
    uint64_t card = wg.getCode();  //записываем код карточки в переменную

    uint64_t n = card;  //создаем временную переменную
    byte bin[40];  //создаем массив для бинарного представления карточки

    uint8_t i = 0;
    while (n > 0) {
      bin[i] = n%2;
      n = n/2;
      i++;
      }            //преобразуем номер карточки из десятичной системы в двоичную (массив получается развернутым)
      
   //записываем двоичный код разделяя по байтам + в разворачиваем массив обратно
    uint8_t byte1 = 0b00000000, byte2 = 0b00000000, byte3 = 0b00000000, byte4 = 0b00000000, byte5 = 0b00000000;
    uint8_t count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0;
   
    for (uint8_t j = 0; j <= i; j++){
      if (count5 < 8) {
        bitWrite(byte5, count5, bitRead(bin[j],0));
        count5++;
        }
      else if (count4 < 8) {
        bitWrite(byte4, count4, bitRead(bin[j],0));
        count4++;
        }
      else if (count3 < 8) {
        bitWrite(byte3, count3, bitRead(bin[j],0));
        count3++;
        }
      else if (count2 < 8) {
        bitWrite(byte2, count2, bitRead(bin[j],0));
        count2++;
        }
      else if (count1 < 8) {
        bitWrite(byte1, count1, bitRead(bin[j],0));
        count1++;
        }
    }
    
    if (button_flag == 1) {
      new_card_write(byte1, byte2, byte3, byte4, byte5);
    }
    else {
      if (eeprom_find_same(byte1, byte2, byte3, byte4, byte5)) { //открываем шкаф
        digitalWrite(RELE_1, HIGH);
        digitalWrite(RELE_2, HIGH);
        digitalWrite(LED_PIN, HIGH);
        delay(RELE_DELAY);
        digitalWrite(LED_PIN, LOW);
        digitalWrite(RELE_1, LOW);
        digitalWrite(RELE_2, LOW);
        }  
    }
	}
 
  if (button_flag == 1){
    delay(BUTTON_DELAY);
    if (digitalRead(BUTTON_PIN) == HIGH) { //долгое нажатие
      button_long_press += BUTTON_DELAY;
      if (button_long_press >= BUTTON_LONG_PRESS_TIME) {
        button_flag = 0;
        button_long_press = 0;
        clear_eeprom();
        }
      }
    else {
      button_long_press = 0;
      }
    }
  
}

uint16_t eeprom_last_adress (void) { //находит последний свободный адрес
  uint16_t last_adress = 0;
  while (!eeprom_5bytes_free(last_adress))
    last_adress = last_adress+5;
  return last_adress; 
}

boolean eeprom_5bytes_free (uint16_t start_number) { //проверяет свободность 5 байтов
  if (start_number > (EEPROM_LENGTH-5))
    return false;
  for (uint8_t i = 0; i < 5; i++) 
    if (EEPROM.read(start_number+i) != 255)
      return false;

  return true;
}

void eeprom_write (uint16_t start_number, uint8_t byte1_, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) { //записывает 5 байт в EEPROM
  EEPROM.update(start_number+0, byte1_);
  EEPROM.update(start_number+1, byte2_);
  EEPROM.update(start_number+2, byte3_);
  EEPROM.update(start_number+3, byte4_);
  EEPROM.update(start_number+4, byte5_);
}

boolean eeprom_find_same(uint8_t byte1_, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) { //ищет такие же карточки в памяти
  uint16_t start_number = 0;
  while(!eeprom_5bytes_free(start_number)) {
    if ((byte1_ != EEPROM.read(start_number+0)) or (byte2_ != EEPROM.read(start_number+1)) or (byte3_ != EEPROM.read(start_number+2)) or (byte4_ != EEPROM.read(start_number+3)) or (byte5_ != EEPROM.read(start_number+4))) {
      start_number = start_number+5;  
      }
    else {
      return true;
      }
    }
  return false;  
}

void new_card_write (uint8_t byte1_, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) { //записывает карточку в память
  if (!eeprom_find_same(byte1_, byte2_, byte3_, byte4_, byte5_)) {
    uint16_t start_number = eeprom_last_adress();
    eeprom_write(start_number, byte1_, byte2_, byte3_, byte4_, byte5_);
    }
  digitalWrite(LED_PIN, LOW);
  button_flag = 0;
}

void clear_eeprom (void) { //чистит EEPROM
  for (int i = 0; i < 1024; i++)
    EEPROM.update(i, 255);
    
  digitalWrite(LED_PIN, LOW);
  for (int i = 0; i < 5; i++) {
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    }
}
  
