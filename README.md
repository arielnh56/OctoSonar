# OctoSonar
Arduino library supports many HC-SR04 sensors via I2C bus and hardware interrupt

This is a rework of the SonarI2C concept, and borrows a lot from it. It is structurally different so I'm starting a new repo.

The main differences are:
* the use of tri-state buffers to mux the echo signals
* the trigger pulse is held down for the duration of each sensor. The falling edge triggers the HC-SR04, and the low value is used to enable the 
matching tri-state buffer to forward the echo signal
* the unit of thing is now the board, not the sensor. No daisy-chaining. That was silly.
