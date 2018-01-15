/* OctoSonarTestLCD
 *  
 *  This is a basic test for the OctoSonar V2+ functionality
 *  outputs range in mm to all 8 sensorts on 1602 or 2004 LCD and optional 
 *  serial text.
 *  
 *  Copyright (c) 2017 Alastair Young
 */
#include "OctoSonar.h"
#include <LiquidCrystal_PCF8574.h>

#define SONAR_ADDR 0x38
#define SONAR_INT 2
#define ACTIVE_SONARS 0x00FF
#define I2C_LCD_ADDRESS 0x27      // default for the current flavor of cheap I2C backpack
//#define LCD_2004                // uncomment for 2004 display with labelled values
//#define OUT_SERIAL              // uncomment for serial output

OctoSonar myOcto(SONAR_ADDR, SONAR_INT);
LiquidCrystal_PCF8574 lcd(I2C_LCD_ADDRESS);

void setup() {
#ifdef OUT_SERIAL
  Serial.begin(115200);
#endif

#ifdef LCD_2004
  lcd.begin(20, 4);              // initialize the lcd
#else
  lcd.begin(16, 2);              // initialize the lcd
#endif
  lcd.setBacklight(255);         // turn on the backlight
  lcd.clear();                   // go home
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
      for (uint8_t i = 0; i < 8; i++) {
        Serial.print(myOcto.read(i)); Serial.print("   ");
      }
      Serial.println();
    }
#endif
  if (last_disp + 200 < millis()) {  // lcd update every 200ms
    last_disp = millis();
#ifdef LCD_2004
    sprintf(buffer, "0 %5d      %5d 7 ", myOcto.read(0), myOcto.read(7));
    lcd.setCursor(0, 0); lcd.print(buffer);
    sprintf(buffer, "1 %5d      %5d 6", myOcto.read(1), myOcto.read(6));
    lcd.setCursor(0, 1); lcd.print(buffer);
    sprintf(buffer, "2 %5d      %5d 5", myOcto.read(2), myOcto.read(5));
    lcd.setCursor(0, 2); lcd.print(buffer);
    sprintf(buffer, "3 %5d      %5d 4", myOcto.read(3), myOcto.read(4));
    lcd.setCursor(0, 3); lcd.print(buffer);
#else
    sprintf(buffer, "%4d%4d%4d%4d", myOcto.read(0), myOcto.read(1), myOcto.read(2), myOcto.read(3));
    lcd.setCursor(0, 0); lcd.print(buffer);
    sprintf(buffer, "%4d%4d%4d%4d", myOcto.read(4), myOcto.read(5), myOcto.read(6), myOcto.read(7));
    lcd.setCursor(0, 1); lcd.print(buffer);
#endif
  }
}



