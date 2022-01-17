/* bench test for OctosonarX2
    for use with test jig
    NOT FOR USE BY END USERS

    (c) Alastair Young 2017
*/

#include <Wire.h>

#define JIGADDR_T 0x21
#define JIGADDR_E 0x22
#define OCTO_ADDR 0x20
#define GREEN_LED 5
#define RED_LED 6
#define INT_PIN 7
#define DEBUG_PIN 8
#define debug(thing) if (digitalRead(DEBUG_PIN) == LOW) {Serial.print(thing);}
#define debugln(thing) if (digitalRead(DEBUG_PIN) == LOW) {Serial.println(thing); delay(50);}
#define debugln2(thing,format) if (digitalRead(DEBUG_PIN) == LOW) {Serial.println(thing, format); delay(50);}
#define OFF HIGH  // pin sink
#define ON LOW
// debug serial chatter all commented out because it seems to interfere with I2C on the jig. Wierd.

void setup() {
  uint8_t i;
  Serial.begin(57600);
  Wire.begin();

  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, OFF);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, OFF);
  pinMode(DEBUG_PIN, INPUT_PULLUP);
  pinMode(INT_PIN, INPUT);

  byte error = 1;
  Serial.println("Looking for test jig T");
  while (error != 0) {
    Wire.beginTransmission(JIGADDR_T);
    error = Wire.endTransmission();
  }
  Serial.println("Found test jig T");
  Wire.beginTransmission(JIGADDR_T);
  Wire.write(0xff); // for read
  Wire.write(0xff); // for read
  error = 1;
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to write to jig T error = ");
    Serial.println(error);
    digitalWrite(RED_LED, ON);
    while (true) {};
  }
  Wire.requestFrom(JIGADDR_T, 2);

  if ( Wire.available() != 2 ) {
    Serial.print("Failed to read from jig T available = ");
    Serial.println(Wire.available());
    digitalWrite(RED_LED, ON);
    while (true) {};
  }
  while (Wire.available()) {
    Wire.read();
  }

  Serial.println("Looking for test jig E");
  while (error != 0) {
    Wire.beginTransmission(JIGADDR_E);
    error = Wire.endTransmission();
  }
  Serial.println("Found test jig E");
  Wire.beginTransmission(JIGADDR_E);
  Wire.write(0xff); // for read
  Wire.write(0xff); // for read
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to write to jig E error = ");
    Serial.println(error);
    digitalWrite(RED_LED, ON);
    while (true) {};
  }
  Wire.requestFrom(JIGADDR_E, 2);
  error = Wire.endTransmission(true);
  if (Wire.available() != 2  ) {
    Serial.print("Failed to read from jig E available = ");
    Serial.println(Wire.available());
    digitalWrite(RED_LED, ON);
    while (true) {};
  }
  while (Wire.available()) {
    Wire.read();
  }
}
void loop() {
  byte i;
  uint16_t retVal;
  byte error = 1;
  // keep last status 5s before start again
  delay(5000);
  digitalWrite(GREEN_LED, OFF);
  digitalWrite(RED_LED, OFF);
  Serial.println("Looking for OctoSonar");
  while (error != 0) {
    Wire.beginTransmission(OCTO_ADDR);
    error = Wire.endTransmission();
  }
  Serial.println("Found OctoSonar");
  digitalWrite(RED_LED, OFF);
  // wait 1s for good pogopin press
  delay(1000);

  // set all TRIGs high
  //  debugln("Set all trigs high");
  Wire.beginTransmission(OCTO_ADDR);
  Wire.write(0xff);
  Wire.write(0xff);
  error = Wire.endTransmission(true);
  if ( error ) {
    Serial.print("Failed to write to OctoSonar error = ");
    Serial.println(error);
    digitalWrite(RED_LED, ON);
    return;
  }
  //read them back
  Wire.requestFrom(JIGADDR_T, 2);
  if (Wire.available() != 2) {
    Serial.print("Error T_READ available =  ");
    Serial.println(Wire.available());
    digitalWrite(RED_LED, ON);
    return;

  }
  retVal += Wire.read();
  retVal += (Wire.read() << 8);

  if (retVal != 0xffff) {
    Serial.print("Error T_READ ");
    Serial.println(retVal, HEX);
    digitalWrite(RED_LED, ON);
    return;
  }
  // debugln("Trigs all look high");

  // set all echos high
  Wire.beginTransmission(JIGADDR_E);
  Wire.write(0xff);
  Wire.write(0xff);
  error = Wire.endTransmission(true);

  // trigs are high, INT should be low from pulldown
  if (digitalRead(INT_PIN) == HIGH) {
    Serial.println("Error: INT is high on idle");
    digitalWrite(RED_LED, ON);
    return;
  }

  for (byte port = 0; port <= 0xf; port++) {
    uint16_t portByte = ~(1 << port);
    uint16_t echoByte = 0xffff;
    debug("Checking port ");
    debugln(port);
    debug("portByte ");
    debugln2(portByte, HEX);
    // set all echos high
    Wire.beginTransmission(JIGADDR_E);
    Wire.write((uint8_t)(echoByte & 0xff));
    Wire.write((uint8_t)((echoByte >> 8) & 0xff));
    error = Wire.endTransmission(true);
    if ( error ) {
      Serial.print("Failed to write to jig E  = ");
      Serial.println(error);
      digitalWrite(RED_LED, ON);
      return;
    }

    // enable the port

    Wire.beginTransmission(OCTO_ADDR);
    Wire.write((uint8_t)(portByte & 0xff));
    Wire.write((uint8_t)((portByte >> 8) & 0xff));
    error = Wire.endTransmission(true);
    if ( error ) {
      Serial.print("Failed to write to OctoSonar error = ");
      Serial.println(error);
      digitalWrite(RED_LED, ON);
      return;
    }
    debug("sent portByte ");
    debugln2(portByte, HEX);
    //read them back
    Wire.requestFrom(JIGADDR_T, 2);
    if (Wire.available() != 2) {
      Serial.print("Error 1 T_READ available =  ");
      Serial.println(Wire.available());
      digitalWrite(RED_LED, ON);
      return;

    }
    retVal = Wire.read();
    retVal += (Wire.read() << 8);
    debug("trigs ");
    debugln2(retVal, HEX);

    if (retVal != portByte) {
      Serial.print("Error 1 - I see trigs ");
      Serial.println(retVal, HEX);
      digitalWrite(RED_LED, ON);
      return;
    }

    // check the pin val - should be high
    while (digitalRead(INT_PIN) == LOW) {
      Serial.print("Error: INT is low when echo is high trig ");
      Serial.println(portByte, HEX);
      Serial.print("echo ");
      Serial.println(echoByte, HEX);
      Serial.println(digitalRead(INT_PIN));
      digitalWrite(RED_LED, ON);
      //return;
    }
    //    debugln("High gives high");

    // drop the other echoes - should have no effect
    echoByte = 0xffff;
    for (i = 0; i <= 0xf; i++) {
      if (i == port) continue; // skip self
      echoByte &= ~(1 << i);
      Wire.beginTransmission(JIGADDR_E);
      Wire.write((uint8_t)(echoByte & 0xff));
      Wire.write((uint8_t)((echoByte >> 8) & 0xff));
      error = Wire.endTransmission(true);
      if ( error ) {
        Serial.print("Failed to write to JIG E error 3 = ");
        Serial.println(error);
        digitalWrite(RED_LED, ON);
        return;
      }

      //read trigs - should not have changed
      Wire.requestFrom(JIGADDR_T, 2);
      retVal = Wire.read();
      retVal += (Wire.read() << 8);
      error = Wire.endTransmission(true);
      if ( error ) {
        Serial.print("Failed to read from jig error 3 = ");
        Serial.println(error);
        digitalWrite(RED_LED, ON);
        return;
      }
      if (retVal != portByte) {
        Serial.print("Error 3 - I see trigs ");
        Serial.print(retVal, HEX);
        Serial.print(" wanted ");
        Serial.println(portByte, HEX);
        digitalWrite(RED_LED, ON);
        return;
      }
      if (digitalRead(INT_PIN) == LOW) {
        Serial.print("Error: INT goes low with pin ");
        Serial.println(i);
        Wire.requestFrom(JIGADDR_T, 2);
        retVal = Wire.read();
        retVal += (Wire.read() << 8);
        error = Wire.endTransmission(true);
        Serial.println(retVal, HEX);
        error = Wire.endTransmission(true);
        if ( error ) {
          Serial.print("Failed to read from jig error = ");
          Serial.println(error);
          digitalWrite(RED_LED, ON);
          return;
        }

        digitalWrite(RED_LED, ON);
        return;
      }
      //     debug("INT ok with low pin ");
      //     debugln(i);
    }
    //   debugln("No interference from other echoes");

    // drop self
    echoByte &= ~(1 << port);
    Wire.beginTransmission(JIGADDR_E);
    Wire.write((uint8_t)(echoByte & 0xff));
    Wire.write((uint8_t)((echoByte >> 8) & 0xff));
    error = Wire.endTransmission(true);
    while (digitalRead(INT_PIN) == HIGH) {
      Serial.println("Error: INT is high when echo is low");
      Serial.print("echo ");
      Serial.println(echoByte, HEX);
      Serial.print("port ");
      Serial.println(portByte, HEX);
        Wire.requestFrom(JIGADDR_T, 2);
        retVal = Wire.read();
        retVal = Wire.read() << 8;
        error = Wire.endTransmission(true);
        Serial.print("I see trigs ");
        Serial.println(retVal, HEX);
        error = Wire.endTransmission(true);
        if ( error ) {
          Serial.print("Failed to read from jig error = ");
          Serial.println(error);
          digitalWrite(RED_LED, ON);
          return;
        }
      digitalWrite(RED_LED, ON);
      // return;
    }
    debugln("Low gives low");
  }

  // if we got here, we passed
  Serial.println("PASS");
  digitalWrite(GREEN_LED, ON);
  digitalWrite(RED_LED, OFF);
}



