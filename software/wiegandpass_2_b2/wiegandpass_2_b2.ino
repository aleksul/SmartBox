#include <Wiegand.h>
#include <EEPROM.h>
WIEGAND wg;

#define button_pin 5
#define led_pin 10

void setup() {
	Serial.begin(9600);  
	wg.begin();
  pinMode(button_pin, INPUT); 
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
}


void loop() {
	if(wg.available())
	{
    uint64_t card = wg.getCode();  //записываем код карточки в переменную
    //Serial.println(card);
		//Serial.println(card, BIN);

/*    String string_card_bin = String(card, BIN);
    Serial.println(string_card_bin);
*/   
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

    Serial.println(byte1, BIN);
    Serial.println(byte2, BIN);
    Serial.println(byte3, BIN);
    Serial.println(byte4, BIN);
    Serial.println(byte5, BIN);

    Serial.println(eeprom_found_last_adress());
	}
}

uint16_t eeprom_found_last_adress (void) { //находит последний свободный адрес
  uint16_t last_adress = 0;
  while (!eeprom_5bytes_free(last_adress))
    last_adress = last_adress+5;
  return last_adress; 
  }

boolean eeprom_5bytes_free (uint16_t start_number) { //проверяет свободность 5 байтов
  for (uint8_t i = 0; i < 5; i++) 
    if (EEPROM.read(start_number+i) != 255)
      return false;

  return true;
  }

void eeprom_write (uint16_t start_number, uint8_t byte1_, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) {
  EEPROM.update(start_number+0, byte1_);
  EEPROM.update(start_number+1, byte2_);
  EEPROM.update(start_number+2, byte3_);
  EEPROM.update(start_number+3, byte4_);
  EEPROM.update(start_number+4, byte5_);
  }

void new_card_write (uint8_t byte1_, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) {
  if (button) {
    
    }
  }
