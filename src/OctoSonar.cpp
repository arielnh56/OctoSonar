/*
  OctoSonar.cpp
  Copyright (c) 2017 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/
#include "OctoSonar.h"
#include <Wire.h>

// constructor
OctoSonar::OctoSonar() 
#ifdef OCTOSONARX2
  : active(0x7777) // default to 12 for dodec
#else
  : active(0xff) // default to 8 for octo2
#endif
  , _address(0x38)        // PCF8574A and PCF8575
  , _interrupt(OCTOSONAR_INT_PIN)
  , _range{}            // last good range
  , _OORcount{}         // last good response time in microseconds - 16 bit gives us up to 65ms
{}

OctoSonar::OctoSonar(uint8_t address, uint8_t interrupt) {
  _address = constrain(address, 0x38, 0x3F);
  _interrupt = interrupt;
}

// static class variables
uint8_t  OctoSonar::maxOOR = 1;                         // how many OOR to skip. Raise this in noisy environments
double   OctoSonar::units = OCTOSONAR_MM;               // defaults to OCTOSONAR_MM 
uint16_t OctoSonar::_max_micros = 4000 / 0.170145;      // limit in microseconds. Above this return zero.
uint8_t  OctoSonar::_phase = OCTOSONAR_PHASE_IDLE;      // what we did last
uint32_t OctoSonar::_pulseBegin = 0;                    // to remember when the pulse started
uint16_t OctoSonar::_spacing = OCTOSONAR_SPACING;       // time between pings - default 50ms
uint32_t OctoSonar::_last_sonar_millis = 0;             // timestamp of last ping sent
uint8_t  OctoSonar::_currentSonar = 0;                  // current active sonar - array index
OctoSonar  *OctoSonar::_currentOctoSonar = 0;           // current active OctoSonar


void OctoSonar::begin() {
  Wire.begin();
  pinMode(_interrupt, INPUT);
  Wire.beginTransmission(_address);
  Wire.write(0xff); // clear the expander
  Wire.endTransmission();

  // insert into circular linked list
  // for the artists that need more than one of these
  if (_currentOctoSonar) {
    _nextOctoSonar = _currentOctoSonar->_nextOctoSonar;
    _currentOctoSonar->_nextOctoSonar = this;
  } else {  // new list
    _currentOctoSonar = this;
    _nextOctoSonar = this;
  }
}

#ifdef OCTOSONARX2
void OctoSonar::begin(uint16_t newactive) {
#else
void OctoSonar::begin(uint8_t newactive) {
#endif
  active = newactive;
  begin();
}

// call from loop() every cycle
void OctoSonar::doSonar() {
  OctoSonar *startOcto = _currentOctoSonar;     // loop catcher for no octos enabled
  uint8_t startSonar =  _currentSonar;          // ditto  
  if (_last_sonar_millis + _spacing < millis()) {  // time to move forward
    if (_phase != OCTOSONAR_PHASE_IDLE) {
      // clean up last sonar - it is not done
      _currentOctoSonar->_OORcount[_currentSonar]++;
      if (_currentOctoSonar->_OORcount[_currentSonar] > maxOOR) {
        _currentOctoSonar->_range[_currentSonar] = 0;
      }
      detachInterrupt(digitalPinToInterrupt(_currentOctoSonar->_interrupt)); // clean up after ourselves
    } 
    // high all triggers
    Wire.beginTransmission(_currentOctoSonar->_address);
#ifdef OCTOSONARX2
    Wire.write(0xffff,2);
#else
    Wire.write(0xff);
#endif
    Wire.endTransmission();
    // look for next enabled sonar
    do {
      _currentSonar++;
      if (_currentSonar >= OCTOSONAR_PORTS ) { // roll to next board
        _currentOctoSonar = _currentOctoSonar->_nextOctoSonar;
	_currentSonar = 0;
      }
      if (startOcto == _currentOctoSonar && startSonar ==  _currentSonar) return;
    } while ((_currentOctoSonar->active & (1 << _currentSonar)) == 0);
  
    _last_sonar_millis = millis();
    // drop the trigger we want
    Wire.beginTransmission(_currentOctoSonar->_address);
#ifdef OCTOSONARX2
    Wire.write(~((uint16_t)1 << _currentSonar),2);
#else
    Wire.write(~((uint8_t)1 << _currentSonar));
#endif
    Wire.endTransmission();

    // check the pin state - it should be down right now, echo takes a while to start 
    if (digitalRead(_currentOctoSonar->_interrupt) == HIGH) { // interrupt pin is active - sensor wedged
      _currentOctoSonar->_range[_currentSonar] = 0;
      return; 
    }
    attachInterrupt(digitalPinToInterrupt(_currentOctoSonar->_interrupt), _startPulse, RISING); // watch for Echo start 
    _phase = OCTOSONAR_PHASE_ECHO_START;
  }
}

void OctoSonar::_startPulse() {
  _pulseBegin = micros(); // pulse is starting now
  attachInterrupt(digitalPinToInterrupt(_currentOctoSonar->_interrupt), _endPulse, FALLING); // now look for pulse end
  _phase = OCTOSONAR_PHASE_ECHO_END;
}

void OctoSonar::_endPulse() {
  uint32_t now = micros();
  detachInterrupt(digitalPinToInterrupt(_currentOctoSonar->_interrupt)); // clean up after ourselves
  // ignore wacko values
  uint32_t pulseLen = now - _pulseBegin; // calculate length of pulse
  if (pulseLen > _max_micros) { // took too long 0 = out of range
    _currentOctoSonar->_OORcount[_currentSonar]++;
    if (_currentOctoSonar->_OORcount[_currentSonar] > maxOOR) {
      _currentOctoSonar->_range[_currentSonar] = 0;
    }
  } else {
    _currentOctoSonar->_OORcount[_currentSonar] = 0;
    _currentOctoSonar->_range[_currentSonar] = pulseLen * units;
  }
  _phase = OCTOSONAR_PHASE_IDLE;
}

// public member functions
int16_t OctoSonar::read(uint8_t sonar) {
  return _range[sonar];
}

int16_t OctoSonar::read(uint8_t sonar, int16_t outOfRange) {
  if (_range[sonar] == 0 ) {
    return outOfRange;
  } else {
    return _range[sonar];
  }
}


