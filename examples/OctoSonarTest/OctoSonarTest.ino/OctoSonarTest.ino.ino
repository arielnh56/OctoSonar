/* OctoSonarX2Test
 *  
 *  This is a basic test for the OctoSonar functionality
 *  outputs range in mm of 8 sensors to serial monitor
 *  serial text at 115200 baud
 *  
 *  Copyright (c) 2018 Alastair Young
 */
#include "OctoSonar.h"

#define SONAR_ADDR 0x38
#define SONAR_INT 2
#define ACTIVE_SONARS 0xFF

OctoSonar myOcto(SONAR_ADDR, SONAR_INT);

void setup() {
  Serial.begin(115200);
  myOcto.begin(ACTIVE_SONARS);   // initialize bus, pins etc
}

uint32_t last_print = 0;
void loop() {
  OctoSonar::doSonar();  // call every cycle, OctoSonar handles the spacing

  if (last_print + 200 < millis()) {   // serial output every 200ms
    last_print = millis();
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(myOcto.read(i)); Serial.print("   ");
    }
    Serial.println();
  }
}




