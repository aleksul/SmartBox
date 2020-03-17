#include <Wiegand.h>
//#include <EEPROM.h>
WIEGAND wg;

int adress = 0;
uint8_t byte1 = 0b00000000, byte2 = 0b00000000, byte3 = 0b00000000, byte4 = 0b00000000, byte5 = 0b00000000;
uint16_t count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0;

void setup() {
	Serial.begin(9600);  
	wg.begin();
}


void loop() {
	if(wg.available())
	{
    uint32_t card = wg.getCode();
    Serial.println(card);
		Serial.println(card, BIN);

/*    String string_card_bin = String(card, BIN);
    Serial.println(string_card_bin);
*/   
    uint32_t n = card;
    byte bin[40];

    int i = 0;
    while (n > 0) {
      bin[i] = n%2;
      n = n/2;
      i++;
      }
    Serial.println(i);
      
    
   
    for (int j = 0; j <= i; j++){
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
	}
}
