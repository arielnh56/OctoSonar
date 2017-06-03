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
//#define OCTOSONAR_DODEC			// future verion with PCF8575
#define OCTOSONAR_PHASE_IDLE		0	// waiting on next window
#define OCTOSONAR_PHASE_ECHO_START	1	// waiting on echo start
#define OCTOSONAR_PHASE_ECHO_END	2	// waiting on echo end
#define OCTOSONAR_MM			0.170145
#define OCTOSONAR_CM			0.0170145
#define OCTOSONAR_IN			0.00669862
#define OCTOSONAR_US			1

class OctoSonar
{
  // user-accessible "public" interface
  public:
    OctoSonar(uint8_t address); // constructor
    void begin();
    void begin(uint8_t interrupt);
    static double units;              // multiplier for unit calculation
#ifdef OCTOSONAR_DODEC
    static uint16_t active;                    // mask of active sensors 
    void begin(uint8_t interrupt, uint16_t active);
#else
    static uint8_t active;                    // mask of active sensors 
    void begin(uint8_t interrupt, uint8_t active);
#endif
    void doSonar();                                   // call every loop()
    int16_t read(uint8_t sonar);
    int16_t OctoSonar::read(uint8_t sonar, int16_t outOfRange);
    static uint8_t maxOOR;                                   // how many OOR to skip. Raise this in noisy environments
    // library-accessible "private" interface
  private:
    uint8_t  _address;                   // PCF8574 0x20 - 0x27 or PCF8574A 0x38 - 0x3F
    uint8_t  _pin;                       // expander pin 0 -7
    static uint16_t _max_micros;                // limit in microseconds. Above this return zero.
#ifdef OCTOSONAR_DODEC
    static uint16_t _micros[16];           // last good response time in microseconds - 16 bit gives us up to 65ms
    static uint8_t  _OORcount[16];         // last good response time in microseconds - 16 bit gives us up to 65ms
#else
    static uint16_t _micros[8];           // last good response time in microseconds - 16 bit gives us up to 65ms
    static uint8_t  _OORcount[8];         // last good response time in microseconds - 16 bit gives us up to 65ms
#endif
    static uint8_t  _currentSonar;          // current active sonar
    static uint8_t _phase; // what we did last
    static uint32_t _pulseBegin;         // to remember when the pulse started
    static uint16_t _spacing;            // time between pings - default 50ms
    static uint8_t  _interrupt;          // interrupt pin - default 2
    static uint32_t _last_sonar_millis;  // timestamp of last ping sent
    void     _send_ping();               // trigger the sensor and set the interrupt
    static void     _startPulse();       // interrupt callout when a pulse starts
    static void     _endPulse();         // interrupt callout when a pulse ends
};

#endif

