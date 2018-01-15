/* OctoSonarX2Test12LCD
 *  
 *  This is a basic test for the OctoSonarX2 functionality
 *  outputs range in mm of 12 sensors on trimounts 1602 to 2004 and 0802 LCD and optional 
 *  serial text.
 *  
 *  Copyright (c) 2017 Alastair Young
 */
#include "OctoSonar.h"
#include <LiquidCrystal_PCF8574.h>

#define SONAR_ADDR 0x20
#define SONAR_INT 2
#define ACTIVE_SONARS 0x7777
#define I2C_LCD_ADDRESS 0x27      // default for the current flavor of cheap I2C backpack
#define LCD_2004                // uncomment for 2004 display with labelled values
#define I2C_LCD2_ADDRESS 0x26      // default for the current flavor of cheap I2C backpack
#define LCD2_0802                // uncomment for 2004 display with labelled values
//#define OUT_SERIAL              // uncomment for serial output

OctoSonarX2 myOcto(SONAR_ADDR, SONAR_INT);
LiquidCrystal_PCF8574 lcd(I2C_LCD_ADDRESS);
LiquidCrystal_PCF8574 lcd2(I2C_LCD2_ADDRESS);

void setup() {
#ifdef OUT_SERIAL
  Serial.begin(115200);
#endif


  lcd.begin(20, 4);              // initialize the lcd
  lcd.setBacklight(255);         // turn on the backlight
  lcd2.begin(8, 2);              // initialize the lcd
  lcd2.setBacklight(255);         // turn on the backlight
  lcd.clear();                   // go home
  lcd2.clear();                   // go home
  myOcto.begin(ACTIVE_SONARS);   // initialize bus, pins etc
}

uint32_t last_print = 0;
uint32_t last_disp = 0;
void loop() {
  char buffer[20];
  OctoSonar::doSonar();  // call every cycle, OctoSonar handles the spacing

#ifdef OUT_SERIAL
    if (last_print + 200 < millis()) {   // serial output every 200ms
      last_print = millis();
      for (uint8_t i = 0; i < 16; i++) {
        Serial.print(myOcto.read(i)); Serial.print("   ");
      }
      Serial.println();
    }
#endif
  if (last_disp + 200 < millis()) {  // lcd update every 200ms
    last_disp = millis();
    sprintf(buffer, "1 %5d      %5d C ", myOcto.read(1), myOcto.read(0xC));
    lcd.setCursor(0, 0); lcd.print(buffer);
    sprintf(buffer, "0 %5d      %5d D", myOcto.read(0), myOcto.read(0xD));
    lcd.setCursor(0, 1); lcd.print(buffer);
    sprintf(buffer, "5 %5d      %5d 8", myOcto.read(5), myOcto.read(8));
    lcd.setCursor(0, 2); lcd.print(buffer);
    sprintf(buffer, "4 %5d      %5d 9", myOcto.read(4), myOcto.read(9));
    lcd.setCursor(0, 3); lcd.print(buffer);

    sprintf(buffer, "%4d%4d", myOcto.read(2), myOcto.read(0xE));
    lcd2.setCursor(0, 0); lcd2.print(buffer);
    sprintf(buffer, "%4d%4d", myOcto.read(6), myOcto.read(0xA));
    lcd2.setCursor(0, 1); lcd2.print(buffer);
  }
}



