/*
  OctoSonar.cpp
  Copyright (c) 2017 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/
#ifndef OCTOSONAR_H
#define OCTOSONAR_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define OCTOSONAR_INT_PIN		2	// default interrupt pin
#define OCTOSONAR_SPACING		50	// millis to avoid echo
#define OCTOSONARX2			// future verion with PCF8575
#ifdef OCTOSONARX2
#define OCTOSONAR_PORTS                 16      // ports per PCF8575
#else
#define OCTOSONAR_PORTS                 8       // ports per PCF8574
#endif
#define OCTOSONAR_PHASE_IDLE		0	// waiting on next window
#define OCTOSONAR_PHASE_ECHO_START	1	// waiting on echo start
#define OCTOSONAR_PHASE_ECHO_END	2	// waiting on echo end
#define OCTOSONAR_MM			0.170145      // TBD integer arithmetic approximations
#define OCTOSONAR_CM			0.0170145
#define OCTOSONAR_IN			0.00669862
#define OCTOSONAR_US			1

class OctoSonar_base
{
  // user-accessible "public" interface
  public:
    // instance stuff
    OctoSonar_base(uint8_t numsonars); // defaults constructor 8, 0x38, 2
    OctoSonar_base(uint8_t numsonars, uint8_t address, uint8_t interrupt); // constructor
    void begin();
    uint16_t active;                    // mask of active sensors 
    void begin(uint16_t active);
    int16_t read(uint8_t sonar);                     
    int16_t read(uint8_t sonar, int16_t outOfRange);
    // class stuff
    static uint8_t maxOOR;               // how many OOR to skip. Raise this in noisy environments
    static double units;                        // defaults to OCTOSONAR_MM 
    static void doSonar();             // call every loop()
    // library-accessible "private" interface
  private:
    // instance stuff
    uint8_t  _address;                   // PCF8574 0x20 - 0x27 or PCF8574A 0x38 - 0x3F
    uint8_t  _interrupt;          // interrupt pin - default 2
    uint16_t *_range;           // last good response time in microseconds - 16 bit gives us up to 65ms
    uint16_t  *_OORcount;         // last good response time in microseconds - 16 bit gives us up to 65ms
    uint8_t _numsonars;            // 8 = PCF8574 or PCF8574A (1 byte comms), 16 = PCF8575 (2 byte comms)
    // class stuff
    static uint16_t _max_micros;         // limit in microseconds. Above this return zero.
    static uint8_t _phase;               // what we did last
    static uint32_t _pulseBegin;         // to remember when the pulse started
    static uint16_t _spacing;            // time between pings - default 50ms
    static uint32_t _last_sonar_millis;  // timestamp of last ping sent
    static uint8_t  _currentSonar;       // current active sonar - array index
    static void     _send_ping();        // trigger the sensor and set the interrupt
    static void     _startPulse();       // interrupt callout when a pulse starts
    static void     _endPulse();         // interrupt callout when a pulse ends
    static OctoSonar_base *_currentOctoSonar; // pointer to the active OctoSonar
    OctoSonar_base *_nextOctoSonar;           // link to the next OctoSonar in the list
};

class OctoSonar :
	public OctoSonar_base::OctoSonar_base {
		public:
			OctoSonar() : OctoSonar_base(8) {}
			OctoSonar(uint8_t address, uint8_t interrupt): OctoSonar_base(8, address, interrupt) {}
	};

class OctoSonarX2 :
	public OctoSonar_base::OctoSonar_base {
		public:
			OctoSonarX2() : OctoSonar_base(16, 0x20, 2) {}
			OctoSonarX2(uint8_t address, uint8_t interrupt): OctoSonar_base(16, address, interrupt) {}
	};

#endif

