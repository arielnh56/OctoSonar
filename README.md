# OctoSonar library for Arduino
Version: 0.0.0<br>
Release date: unreleased<br>
LINKS HERE

## Summary

This is a library for the Arduino IDE that allows the polling of multiple ultrasonic distance sensors using the I2C bus and a single hardware interrupt pin. It assumes a PCF8574(A) type port expander to trigger the sensors, and tri-state buffers to multiplex the echo signals. It is specifically developed and maintained to support the author's OctoSonar boards version 2.0+. For earlier boards see the SonarI2C library.

It has been developed for use with the HC-SR04 sensor - and has been tested with units from moultiple manufacturers.

## Supported Platforms

This library is designed to work with the Arduino IDE versions 1.6.x or later; it is not tested it with earlier versions. It should work with any Arduino compatible board with an available hardware interrupt pin. Note that other interrupt activity may interfere with the accuracy and reliability of this code.

## Getting Started

### Hardware

This code supports the OctoSonar v2.0 and later. Schematics and board designs are included in this repo.

The board should be connected to your controller as follows:

* GND -> GND
* VCC -> VCC (5V on the UNO)
* SDA -> SDA (A4 in the UNO)
* SCL -> SCL (A5 on the UNO)
* INT -> a hardware interrupt pin (2 or 3 on the UNO)
* 5V -> a 5V supply (VCC on the UNO)

Note that the 5V supply drives the sensors and the VCC drives the electronics. This should allow the unit to be supported by 3.3V controllers (some other Arduino formats and Raspberry Pi). This 3.3V mode is not yet tested and validated - USE AT YOUR OWN RISK. If you want to save a wire with a 5V controller there is a solder jumper on the back of the board to connect the 5V and VCC pins together.

As manufactured the board has an I2C address of 0x38. This can be altered via solder jumpers on the back.

### Software

If you are using version 1.6.2 or later of the [Arduino software (IDE)](http://www.arduino.cc/en/Main/Software), you can use the Library Manager to install this library:

1. In the Arduino IDE, open the "Sketch" menu, select "Include Library", then "Manage Libraries...".
2. Search for "OctoSonar".
3. Click the OctoSonar entry in the list.
4. Click "Install".

If this does not work, you can manually install the library:

1. Download the [latest release archive from GitHub](https://github.com/arielnh56/OctoSonar/releases) and decompress it.
2. Rename the folder "OctoSonar-master" to "OctoSonar".
3. Move the "OctoSonar" folder into the "libraries" directory inside your Arduino sketchbook directory.  You can view your sketchbook location by opening the "File" menu and selecting "Preferences" in the Arduino IDE.  If there is not already a "libraries" folder in that location, you should make the folder yourself.
4. After installing the library, restart the Arduino IDE.

(note - the above instructions adapted from a Pololu readme)

## Examples

An example is included showing the use of the OctoSonar with 8 sensors. It will display the 8 measurement on an LCDand the serial monitor. It supports 1602 and 2004 LCDs.
## Library reference

* Constructor. Call once outside loop() and setup(). This sets the I2C address of the pin expander (0x38 - 0x3F).

 ```c
    OctoSonar(uint8_t address); // constructor
```

* call one of the begin() functions from setup(). This sets the interrupt pin to use.

 ```c
    static void begin();                                     // call from setup(). Defaults to pin 2 and 50ms
    static void begin(uint8_t interrupt);                    // same thing but set the pin
```

* by default we skip one out of range/failed echo,keeping the last value. It may be useful to raise this count in sub-optimal echo environments.

 ```c
    static uint8_t maxOOR;                                   // how many OOR to skip. Raise this in noisy environments
```

* by default the output is in mm. You can change this by setting this to OCTOSONAR_CM for cm, or OCTOSONAR_IN for inches. It is the value needed to conver from microseconds to the desired unit.

 ```c
    static double units;                        // defaults to OCTOSONAR_MM 
```

*  Disabling sensors that you are not interested in right now allows the other sensors to poll more often. This is a bitmask mapping to the 8 sensors on the unit. A '1' is 'active'.

 ```c
    static uint16_t active;                    // mask of active sensors 
```

* Call this every loop(). If it is time, it will poll the next enabled sensor.

 ```c
    static void doSonar();
```

* Return values. By default, when the sensor does not get a good echo a zero value is returned. Using the second form of read() allows you to set you own preference for what is returned. The returned value defaults to mm.
 ```c
    int16_t read(uint8_t sonar);                     
    int16_t read(uint8_t sonar, int16_t outOfRange);
```
 
## Relationship to SonarI2C

This is a rework of the SonarI2C concept, and borrows a lot from it. It is structurally different so I'm starting a new repo.

The main differences are:
* the use of tri-state buffers to mux the echo signals
* the trigger pulse is held down for the duration of each sensor. The falling edge triggers the HC-SR04, and the low value is used to enable the 
matching tri-state buffer to forward the echo signal
* the unit of thing is now the board, not the sensor. No daisy-chaining. That was silly.

## Version history

* 0.0.0 (DATE HERE): not released.

