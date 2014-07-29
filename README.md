ButtonPad
=========

This is the code to drive a Sparkfun Button Pad (4x4) using an arduino, and using i2c (Wire) to recieve display commands.
The reason to make the driving arduino an i2c slave, and have another arduino do the main logic is because driving the pad takes up almost ALL pins on the 328 based arduinos.


Required Libraries - Driver/Slave
---------------------------------
* [TLC5940](https://github.com/hideyukisaito/Tlc5940)
* [Keypad](https://github.com/Nullkraft/Keypad)

Required Libraries - Master
---------------------------
* Nothing, just this library.


Installation - Driver/Slave
---------------------------
* Wire up the Tlc5940 as per the library documentation
* Connect the pad's RGB source pins to the start of the Tlc5940 (OUT 0-11)
* Wire up the PNP transistors to the remaining Tlc5940 outputs (OUT 12-15)
* Connecto the keypad to the arduino's pins A0, A1, A2, A3, D5, D6, D7, D8
* Connect up the Slave arduino to the Master using the I2C wires (A4, A5)
* Connect the IRQ line, 4 on the Slave, to any pin on the master.
* Open up examples/ButtonPadFirmware.ino in the IDE and upload!!!

Installation - Master
---------------------
* Connect the I2C bus to the slave
* Connect the IRQ/INT pin to pin 4 on the Slave.
* Upload the LED example and go!
