/* bench test for Octosonar V2.x
    for use with test jig on new boards

    (c) Alastair Young 2017
*/

#include <Wire.h>

#define JIGADDR 0x20
#define OCTO_ADDR 0x38
#define GREEN_LED 13
#define RED_LED 12
#define INT_PIN 11
#define DEBUG_PIN 10
#define debug(thing) if (digitalRead(DEBUG_PIN) == LOW) {Serial.print(thing);}
#define debugln(thing) if (digitalRead(DEBUG_PIN) == LOW) {Serial.println(thing); delay(50);}
#define debugln2(thing,format) if (digitalRead(DEBUG_PIN) == LOW) {Serial.println(thing, format); delay(50);}
// debug serial chatter all commented out because it seems to interfere with I2C on the jig. Wierd.

uint8_t echoPin[8] = { 2, 3, 4, 5, A0, A1, A2, A3 };
void setup() {
  uint8_t i;
  Serial.begin(9600);
  Wire.begin();
  for (i = 0; i <= 7; i++) {
    pinMode(echoPin[i], OUTPUT);
    digitalWrite(echoPin[i], LOW);
  }

  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
  pinMode(DEBUG_PIN, INPUT_PULLUP);

  byte error = 1;
  Serial.println("Looking for test jig");
  while (error != 0) {
    Wire.beginTransmission(JIGADDR);
    error = Wire.endTransmission();
  }
  Serial.println("Found test jig");
  Wire.beginTransmission(JIGADDR);
  Wire.write(0xff); // for read
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to write to jig error = ");
    Serial.println(error);
    digitalWrite(RED_LED, HIGH);
    while (true) {};
  }
  Wire.requestFrom(JIGADDR, 1);
  Wire.read(); // set to read mode
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to read from jig error = ");
    Serial.println(error);
    digitalWrite(RED_LED, HIGH);
    while (true) {};
  }
  Wire.requestFrom(JIGADDR, 1);
  Wire.read(); // set to read mode
  Wire.endTransmission();

}
void loop() {
  byte i;
  byte retVal;
  byte error = 1;
  // keep last status 5s before start again
  delay(5000);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  Serial.println("Looking for OctoSonar");
  while (error != 0) {
    Wire.beginTransmission(OCTO_ADDR);
    error = Wire.endTransmission();
  }
  Serial.println("Found OctoSonar");
  digitalWrite(RED_LED, LOW);
  // wait 1s for good pogopin press
  delay(1000);

  // set all TRIGs high
  //  debugln("Set all trigs high");
  Wire.beginTransmission(OCTO_ADDR);
  Wire.write(0xff);
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to write to OctoSonar error = ");
    Serial.println(error);
    digitalWrite(RED_LED, HIGH);
    return;
  }
  //read them back
  Wire.requestFrom(JIGADDR, 1);
  retVal = Wire.read();
  Wire.endTransmission();

  if (retVal != 0xff) {
    Serial.print("Error 2 - I see trigs ");
    Serial.println(retVal, HEX);
    digitalWrite(RED_LED, HIGH);
    return;
  }
  // debugln("Trigs all look high");

  // set all echos high
  for (i = 0; i <= 7; i++) {
    digitalWrite(echoPin[i], HIGH);
  }

  // trigs are high, INT should be low from pulldown
  if (digitalRead(INT_PIN) == HIGH) {
    Serial.println("Error: INT is high on idle");
    digitalWrite(RED_LED, HIGH);
    return;
  }

  for (byte port = 0; port <= 7; port++) {
    byte portByte = ~(1 << port);
    //  debug("Checking port ");
    //  debugln(port);
    //  debug("portByte ");
    //  debugln2(portByte, HEX);
    // set all echos high
    for (i = 0; i <= 7; i++) {
      digitalWrite(echoPin[i], HIGH);
    }
    // enable the port

    Wire.beginTransmission(OCTO_ADDR);
    Wire.write(portByte);
    error = Wire.endTransmission(true);
    if ( error ) {
      Serial.print("Failed to write to OctoSonar error = ");
      Serial.println(error);
      digitalWrite(RED_LED, HIGH);
      return;
    }
    //  debug("sent portByte ");
    //  debugln2(portByte, HEX);
    //read them back
    Wire.requestFrom(JIGADDR, 1);
    retVal = Wire.read();
    error = Wire.endTransmission(true);
    if ( error ) {
      Serial.print("Failed to read from jig error = ");
      Serial.println(error);
      digitalWrite(RED_LED, HIGH);
      return;
    }
    if (retVal != portByte) {
      Serial.print("Error 1 - I see trigs ");
      Serial.println(retVal, HEX);
      digitalWrite(RED_LED, HIGH);
      return;
    }
    //    debug("trigs ");
    //  debugln2(retVal, HEX);

    // check the pin val - should be high
    if (digitalRead(INT_PIN) == LOW) {
      Serial.println("Error: INT is low when echo is high");
      digitalWrite(RED_LED, HIGH);
      return;
    }
    //    debugln("High gives high");

    // drop the other trigs - should have no effect
    for (i = 0; i <= 7; i++) {
      if (i == port) continue; // skip self
      digitalWrite(echoPin[i], LOW);
      //read them back
      Wire.requestFrom(JIGADDR, 1);
      retVal = Wire.read();
      error = Wire.endTransmission(true);
      if ( error ) {
        Serial.print("Failed to read from jig error = ");
        Serial.println(error);
        digitalWrite(RED_LED, HIGH);
        return;
      }
      if (retVal != portByte) {
        Serial.print("Error 3 - I see trigs ");
        Serial.println(retVal, HEX);
        digitalWrite(RED_LED, HIGH);
        return;
      }
      if (digitalRead(INT_PIN) == LOW) {
        Serial.print("Error: INT goes low with pin ");
        Serial.println(i);
        Wire.requestFrom(JIGADDR, 1);
        Serial.println(Wire.read(), HEX);
        error = Wire.endTransmission(true);
        if ( error ) {
          Serial.print("Failed to read from jig error = ");
          Serial.println(error);
          digitalWrite(RED_LED, HIGH);
          return;
        }

        digitalWrite(RED_LED, HIGH);
        return;
      }
      //     debug("INT ok with low pin ");
      //     debugln(i);
    }
    //   debugln("No interference from other echoes");

    // drop self
    digitalWrite(echoPin[port], LOW);
    if (digitalRead(INT_PIN) == HIGH) {
      Serial.println("Error: INT is high when echo is low");
      digitalWrite(RED_LED, HIGH);
      return;
    }
    //   debugln("Low gives low");
  }

  // if we got here, we passed
  Serial.println("PASS");
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
}



