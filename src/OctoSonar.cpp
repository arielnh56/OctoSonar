/*
  OctoSonar.cpp
  Copyright (c) 2017 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/
#include "OctoSonar.h"
#include <Wire.h>

// constructor
OctoSonar::OctoSonar(uint8_t address) {
  _address = (address < 0x38) ? constrain(address, 0x20, 0x27) : constrain(address, 0x38, 0x3F);
  _max_micros = 4000 / 0.170145; // constrain to spec sheet
}

void OctoSonar::begin() {
  Wire.begin();
  pinMode(_interrupt, INPUT);
  Wire.beginTransmission(_address);
  Wire.write(0xff); // clear the expander
  Wire.endTransmission();
}

void OctoSonar::begin(uint8_t interrupt) {
  _interrupt = interrupt;
  begin();
}

#ifdef OCTOSONAR_DODEC
void OctoSonar::begin(uint8_t interrupt, uint16_t newactive) {
#else
void OctoSonar::begin(uint8_t interrupt, uint8_t newactive) {
#endif
  _interrupt = interrupt;
  active = newactive;
  begin();
}

// call from loop() every cycle
void OctoSonar::doSonar() {
  if (active == 0) return; // no sonars initiated
  if (_last_sonar_millis + _spacing < millis()) {
    if (_phase != OCTOSONAR_PHASE_IDLE) {
      // clean up last sonar - it is not done
      _OORcount[_currentSonar]++;
      if (_OORcount[_currentSonar] > maxOOR) {
        _micros[_currentSonar] = 0;
      }
      detachInterrupt(digitalPinToInterrupt(_interrupt)); // clean up after ourselves
    } 
    // look for next enabled sonar
    do {
#ifdef OCTOSONAR_DODEC
      _currentSonar = (_currentSonar + 1) % 16;
#else
      _currentSonar = (_currentSonar + 1) % 8;
#endif
    } while ((active & (1 << _currentSonar)) == 0);
  
    _last_sonar_millis = millis();
    _send_ping();
  }
}

void OctoSonar::_send_ping() {
    Wire.beginTransmission(_address);
    // high alltriggers
#ifdef OCTOSONAR_DODEC
    Wire.write(0xffff,2);
    Wire.endTransmission();
    // drop the trigger we want
    Wire.beginTransmission(_address);
    Wire.write(~((uint16_t)1 << _currentSonar),2);
#else
    Wire.write(0xff);
    Wire.endTransmission();
    // drop the trigger we want
    Wire.beginTransmission(_address);
    Wire.write(~((uint8_t)1 << _currentSonar));
//    Serial.println((uint8_t)(~((uint8_t)1 << _currentSonar)));

#endif
    Wire.endTransmission();

    // check the pin state - it should be down right now, echo takes a while to start 
  if (digitalRead(_interrupt) == HIGH) { // interrupt pin is active - sensor wedged
    _micros[_currentSonar] = 0;
    return; 
  }
  attachInterrupt(digitalPinToInterrupt(_interrupt), _startPulse, RISING); // watch for Echo start 
  _phase = OCTOSONAR_PHASE_ECHO_START;
}

void OctoSonar::_startPulse() {
  _pulseBegin = micros(); // pulse is starting now
  attachInterrupt(digitalPinToInterrupt(_interrupt), _endPulse, FALLING); // now look for pulse end
  _phase = OCTOSONAR_PHASE_ECHO_END;
}

void OctoSonar::_endPulse() {
  uint32_t now = micros();
  detachInterrupt(digitalPinToInterrupt(_interrupt)); // clean up after ourselves
  // ignore wacko values
  uint32_t pulseLen = now - _pulseBegin; // calculate length of pulse
  if (pulseLen > _max_micros) { // took too long 0 = out of range
    _OORcount[_currentSonar]++;
    if (_OORcount[_currentSonar] > maxOOR) {
      _micros[_currentSonar] = 0;
    }
  } else {
    _OORcount[_currentSonar] = 0;
    _micros[_currentSonar] = pulseLen * units;
  }
  _phase = OCTOSONAR_PHASE_IDLE;
}

// public member functions
int16_t OctoSonar::read(uint8_t sonar) {
  return _micros[sonar];
}

int16_t OctoSonar::read(uint8_t sonar, int16_t outOfRange) {
  if (_micros[sonar] == 0 ) {
    return outOfRange;
  } else {
    return _micros[sonar];
  }
}

// declare private static class variables
uint8_t OctoSonar::_currentSonar = 0;
uint32_t OctoSonar::_last_sonar_millis = 0;
uint32_t OctoSonar::_pulseBegin = 0;
uint8_t OctoSonar::_interrupt = OCTOSONAR_INT_PIN;
uint16_t OctoSonar::_spacing = OCTOSONAR_SPACING;
uint8_t OctoSonar::maxOOR = 1;
uint8_t OctoSonar::_phase = OCTOSONAR_PHASE_IDLE;
double OctoSonar::units = OCTOSONAR_MM; 
uint16_t OctoSonar::_max_micros;                // limit in microseconds. Above this return zero.
#ifdef OCTOSONAR_DODEC
uint16_t OctoSonar::_micros[16] = {};           // last good response time in microseconds - 16 bit gives us up to 65ms
uint8_t  OctoSonar::_OORcount[16];         // last good response time in microseconds - 16 bit gives us up to 65ms
uint8_t  OctoSonar::active = 0x7777; // default to 12 for dodec
#else
uint16_t OctoSonar::_micros[8] = {};           // last good response time in microseconds - 16 bit gives us up to 65ms
uint8_t  OctoSonar::_OORcount[8];         // last good response time in microseconds - 16 bit gives us up to 65ms
uint8_t  OctoSonar::active = 0xff; // default to 8 for octo2
#endif
