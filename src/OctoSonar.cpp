/*
  OctoSonar.cpp
  Copyright (c) 2017 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/
#include "OctoSonar.h"
#include <Wire.h>

// constructor
OctoSonar_base::OctoSonar_base(uint8_t numsonars) 
  : active(0xff) // default to 8 
  , _address(0x38)        // PCF8574A OctoSonar v2
  , _interrupt(OCTOSONAR_INT_PIN)
  , _numsonars(8)  // PCF8574(A)
{
  _numsonars = numsonars;
  _range = new uint16_t[_numsonars]();
  _OORcount = new uint16_t[_numsonars]();
}

OctoSonar_base::OctoSonar_base(uint8_t numsonars, uint8_t address, uint8_t interrupt) {
  _numsonars = numsonars;
  if (numsonars == 8) {
    _address = constrain(address, 0x38, 0x3F);
  } else {
    _address = constrain(address, 0x20, 0x27);
  }
  _interrupt = interrupt;
  _range = new uint16_t[_numsonars]();
  _OORcount = new uint16_t[_numsonars]();
}

// static class variables
uint8_t  OctoSonar_base::maxOOR = 1;                         // how many OOR to skip. Raise this in noisy environments
double   OctoSonar_base::units = OCTOSONAR_MM;               // defaults to OCTOSONAR_MM 
uint16_t OctoSonar_base::_max_micros = 4000 / 0.170145;      // limit in microseconds. Above this return zero.
uint8_t  OctoSonar_base::_phase = OCTOSONAR_PHASE_IDLE;      // what we did last
uint32_t OctoSonar_base::_pulseBegin = 0;                    // to remember when the pulse started
uint16_t OctoSonar_base::_spacing = OCTOSONAR_SPACING;       // time between pings - default 50ms
uint32_t OctoSonar_base::_last_sonar_millis = 0;             // timestamp of last ping sent
uint8_t  OctoSonar_base::_currentSonar = 0;                  // current active sonar - array index
OctoSonar_base  *OctoSonar_base::_currentOctoSonar = 0;           // current active OctoSonar


void OctoSonar_base::begin() {
  Wire.begin();
  pinMode(_interrupt, INPUT);
  Wire.beginTransmission(_address);
  Wire.write(0xff); // clear the expander
  if (_numsonars == 16) {
    Wire.write(0xff);
  }
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

void OctoSonar_base::begin(uint16_t newactive) {
  active = newactive;
  begin();
}

// call from loop() every cycle
void OctoSonar_base::doSonar() {
  OctoSonar_base *startOcto = _currentOctoSonar;     // loop catcher for no octos enabled
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
    Wire.write(0xff);
    if (_currentOctoSonar->_numsonars == 16) {
      Wire.write(0xff);
    }
    Wire.endTransmission();
    // look for next enabled sonar
    do {
      _currentSonar++;
      if (_currentSonar >= _currentOctoSonar->_numsonars ) { // roll to next board
        _currentOctoSonar = _currentOctoSonar->_nextOctoSonar;
	_currentSonar = 0;
      }
      if (startOcto == _currentOctoSonar && startSonar ==  _currentSonar) return;
    } while ((_currentOctoSonar->active & (1 << _currentSonar)) == 0);
  
    _last_sonar_millis = millis();
    // drop the trigger we want
    Wire.beginTransmission(_currentOctoSonar->_address);
    if (_currentOctoSonar->_numsonars == 16) {
      uint16_t trig;
      trig = ~((uint16_t)1 << _currentSonar);
      Wire.write((uint8_t *)&trig,2);
    } else {
      Wire.write(~((uint8_t)1 << _currentSonar));
    }
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

void OctoSonar_base::_startPulse() {
  _pulseBegin = micros(); // pulse is starting now
  attachInterrupt(digitalPinToInterrupt(_currentOctoSonar->_interrupt), _endPulse, FALLING); // now look for pulse end
  _phase = OCTOSONAR_PHASE_ECHO_END;
}

void OctoSonar_base::_endPulse() {
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
int16_t OctoSonar_base::read(uint8_t sonar) {
  return _range[sonar];
}

int16_t OctoSonar_base::read(uint8_t sonar, int16_t outOfRange) {
  if (_range[sonar] == 0 ) {
    return outOfRange;
  } else {
    return _range[sonar];
  }
}


