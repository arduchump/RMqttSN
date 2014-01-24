MQTT-SN Arduino
===============
An implementation of the [MQTT-SN] client protocol for AVR-based
microcontrollers.

Building
--------
Requirements:

If you're using the Arduino IDE, you can just copy the .cpp and .h files in the
src directory to the Arduino libraries folder and it all should magically work.

If you're cross-compiling from the command line, you'll need:

 - CMake >= 2.6
 - gcc-avr
 - avr-libc
 - avrdude
 - binutils-avr
 - core Arduino libraries. These can be obtained from the arduino-core package
   on most \*NIX systems, or as part of the standard Arduino IDE installation.

Run `ccmake` and set `ARDUINO_LIBRARY_PATH` to the `hardware/arduino` path. For
instance on Debian-based systems it is `/usr/share/arduino/hardware/arduino`

The other default values are set for an Arduino [Uno] or a [JeeNode], and will
probably work on most other ATmega328p based hardware.

Run make and it will generate a static library file that can be linked against
in your main project.

Usage
-----
To use UART serial comms, implement `void serialEvent()` - which is part of the
standard Arduino library and gets called whenever there is incoming data on the
serial line - and call `MQTTSN::parse_stream()`

Subclass MQTTSN and override some, all, or none of the handler virtual
functions to get data from the broker or to change application state.

Most of the handler functions are stubs, but a few of them deal with return
code checking, topic registration and so on, so you should call the base class
implementation of the handler before any custom code.

Virtual functions seem to take significant space in the compiled binary, so if
you want to shave some bytes off your final build, comment out the handlers
you're not using in `mqttsn-messages.h` and in `void MQTTSN::dispatch()`

[MQTT-SN]:http://mqtt.org
[Uno]:http://arduino.cc/en/Main/arduinoBoardUno
[JeeNode]:http://jeelabs.net
