
#include "OctoSonar.h"
#include <LiquidCrystal_PCF8574.h>

#define SONAR_ADDR 0x38
#define SONAR_INT 2
#define ACTIVE_SONARS 0x00FF
#define I2C_LCD_ADDRESS 0x27      // default for the current flavor of cheap I2C backpack
OctoSonar myOcto(SONAR_ADDR);
LiquidCrystal_PCF8574 lcd(I2C_LCD_ADDRESS);

void setup() {
  // Serial.begin(115200);
  lcd.begin(20, 4);              // initialize the lcd
  lcd.setBacklight(255);         // turn on the backlight
  lcd.clear();                   // go home
  myOcto.begin(SONAR_INT, ACTIVE_SONARS);   // initialize bus, pins etc
}

uint32_t last_print = 0;
void loop() {
  char buffer[20];
  myOcto.doSonar();  // call every cycle, OctoSonar handles the spacing

  /*
    if (last_print + 100 < millis()) {
      last_print = millis();
      for (uint8_t i = 0; i < 8; i++) {
        Serial.print(myOcto.read(i)); Serial.print("   ");
      }
      Serial.println();
    }
  */
  if (last_print + 500 < millis()) {
    last_print = millis();
    sprintf(buffer, "0 %5d      %5d 7 ", myOcto.read(0), myOcto.read(7));
    lcd.setCursor(0, 0); lcd.print(buffer);
    sprintf(buffer, "1 %5d      %5d 6", myOcto.read(1), myOcto.read(6));
    lcd.setCursor(0, 1); lcd.print(buffer);
    sprintf(buffer, "2 %5d      %5d 5", myOcto.read(2), myOcto.read(5));
    lcd.setCursor(0, 2); lcd.print(buffer);
    sprintf(buffer, "3 %5d      %5d 4", myOcto.read(3), myOcto.read(4));
    lcd.setCursor(0, 3); lcd.print(buffer);
  }
}



