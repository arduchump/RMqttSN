MQTT-SN Arduino
===============
An implementation of the [MQTT-SN] client protocol for AVR-based
microcontrollers.

Building
--------
Requirements:

 - CMake >= 2.6
 - gcc-avr
 - avr-libc
 - avrdude
 - binutils-avr
 - core Arduino libraries. These can be obtained from the arduino-core package
   on most \*NIX systems, or as part of the standard Arduino IDE installation.

Run `ccmake` and set `ARDUINO_LIBRARY_PATH` to the `hardware/arduino` path. For
instance on Debian-based systems it is `/usr/share/ardunio/hardware/arduino`

The other default values are set for an Arduino [Uno] or a [JeeNode].

Usage
-----
Implement `void serialEvent()` - which is part of the standard Arduino library
and gets called whenever there is incoming data on the serial line - and call
`MQTTSN::parse_stream()`

Subclass MQTTSN and override some all or none of the handler virtual functions
to get data from the broker or to change application state.

Most of the handler functions are stubs, but a few of them deal with return
code checking, topic registration and so on, so you should call the base class
implementation of the handler before any custom code.

[MQTT-SN]:http://mqtt.org
[Uno]:http://arduino.cc/en/Main/arduinoBoardUno
[JeeNode]:http://jeelabs.net
