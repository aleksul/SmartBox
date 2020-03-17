#include <Wiegand.h>
#include <EEPROM.h>
WIEGAND wg;

#define BUZZER_PIN 9
#define BUTTON_PIN 7
#define LED_PIN 4
#define EEPROM_LENGTH 1024
#define RELE_1 5
#define RELE_2 LED_BUILTIN
#define RELE_DELAY 5000
#define BUTTON_DELAY 100
#define BUTTON_LONG_PRESS_TIME 10000
#define BUZZER_PIP_TIME 1000
#define BUTTON_RESET_TIME 30000

uint8_t button_flag = 0;
uint8_t mastercard_flag = 0;
uint8_t delete_flag = 0;
uint16_t button_flag_reset_count = 0;
uint16_t button_long_press = 0;

void setup() { 
	wg.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELE_1, OUTPUT);
  pinMode(RELE_2, OUTPUT);
  delay(1000);
  digitalWrite(RELE_1, HIGH);
  digitalWrite(RELE_2, HIGH);
  digitalWrite(LED_PIN, HIGH);
}


void loop() {
  
  if(button_flag == 0 && digitalRead(BUTTON_PIN) == HIGH){
    delay(100); //для того чтобы избежать фантомных нажатий
    if(digitalRead(BUTTON_PIN) == HIGH) {
      button_flag = 1;
      digitalWrite(LED_PIN, LOW);
      }
  }
/*--------------------------------------------------------*/   
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

    if (eeprom_find_same(byte1, byte2, byte3, byte4, byte5) && button_flag == 0 && mastercard_flag == 0 && delete_flag == 0){ //открываем шкаф, если конечно кнопка не была нажата (и другие флаги не стоят)        
      digitalWrite(RELE_1, LOW);
      digitalWrite(RELE_2, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(RELE_DELAY);
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(RELE_1, HIGH);
      digitalWrite(RELE_2, HIGH);      
      }
      
    if (delete_flag == 1){
      //если после удержания кнопки приложили мастер карту, то стираем все
      if ((byte1 == EEPROM.read(0) && byte2 == EEPROM.read(1) && byte3 == EEPROM.read(2) && byte4 == EEPROM.read(3) && byte5 == EEPROM.read(4)) || (byte1 == EEPROM.read(5) && byte2 == EEPROM.read(6) && byte3 == EEPROM.read(7) && byte4 == EEPROM.read(8) && byte5 == EEPROM.read(9))){
        clear_eeprom();
      }
      
      delete_flag = 0;
      mastercard_flag = 0;
      button_flag = 0;
      digitalWrite(LED_PIN, HIGH);  
      }
        
    if (mastercard_flag == 1){ //Не важно, была ли нажата кнопка, если флаг уже поставили 
      mastercard_flag = 0;
      button_flag = 0; //обнулим, а то вдруг ее заново нажали
      new_card_write(byte1, byte2, byte3, byte4, byte5);
      digitalWrite(LED_PIN, HIGH);    
      } 
             
    if (button_flag == 1) { //кнопка была нажата перед тем как приложили карту?
      //А это карточка одна из первых двух?
      if ((byte1 == EEPROM.read(0) && byte2 == EEPROM.read(1) && byte3 == EEPROM.read(2) && byte4 == EEPROM.read(3) && byte5 == EEPROM.read(4)) || (byte1 == EEPROM.read(5) && byte2 == EEPROM.read(6) && byte3 == EEPROM.read(7) && byte4 == EEPROM.read(8) && byte5 == EEPROM.read(9))){
        mastercard_flag = 1; //если да, то поставим флажочек и подмигнем
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(1000);
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        }
      else if (eeprom_5bytes_free(0) || eeprom_5bytes_free(5)){ //а если карточка не подошла, ибо там их вообще нет?
        new_card_write(byte1, byte2, byte3, byte4, byte5);
        digitalWrite(LED_PIN, HIGH);
        }
      else {
        digitalWrite(LED_PIN, HIGH);
        }
      button_flag = 0; //как бы там ни было, кнопку надо сбросить      
      }
	}
/*-----------------Конец работы с карточкой----------------------*/
/*-------------Проверка кнопки на долгое нажатие-----------------*/ 
  if (button_flag == 1){
    delay(BUTTON_DELAY);
    reset_new_card_mode();
    if (digitalRead(BUTTON_PIN) == HIGH) {
      button_long_press += BUTTON_DELAY;
      if (button_long_press%BUZZER_PIP_TIME==0){ //пищим каждый заданный промежуток
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        }
      if (button_long_press >= BUTTON_LONG_PRESS_TIME) {
        button_flag = 0;
        button_long_press = 0;
        digitalWrite(BUZZER_PIN, HIGH);
        delay(1000);
        digitalWrite(BUZZER_PIN, LOW);
        delete_flag = 1;        
        }
      }
    else {
      button_long_press = 0;
      }
    }
/*-------если память пустая, то хорошая идея открыть замки--------*/
  if (eeprom_last_adress()==0) {
    digitalWrite(RELE_1, LOW);
    digitalWrite(RELE_2, LOW);
    }
  else {
    digitalWrite(RELE_1, HIGH);
    digitalWrite(RELE_2, HIGH);      
    }
/*-----------------------Конец  LOOP-----------------------------*/ 
}
/*----------------------Далее - Функции--------------------------*/ 
uint16_t eeprom_last_adress (void) { //находит последний свободный адрес
  uint16_t last_adress = 0;
  while (!eeprom_5bytes_free(last_adress))
    last_adress += 5;
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
    if ((byte1_ != EEPROM.read(start_number+0)) || (byte2_ != EEPROM.read(start_number+1)) || (byte3_ != EEPROM.read(start_number+2)) || (byte4_ != EEPROM.read(start_number+3)) || (byte5_ != EEPROM.read(start_number+4))) {
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
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    }
}

void clear_eeprom (void) { //чистит EEPROM
  for (int i = 0; i < 1024; i++)
    EEPROM.update(i, 255);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);  
  digitalWrite(LED_PIN, HIGH);
  for (int i = 0; i < 5; i++) {
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    }
  
}

void reset_new_card_mode (void) { //bug fix. сбрасывает режим записи новой карты через заданное время
  button_flag_reset_count += BUTTON_DELAY;
  if (button_flag_reset_count == BUTTON_RESET_TIME) {
    button_flag_reset_count = 0;
    button_flag = 0;
    digitalWrite(LED_PIN, HIGH);
    }
  }
  
