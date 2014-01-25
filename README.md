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
To use UART serial comms, define `USE_SERIAL` on the command line (or if you're
using the Arduino IDE, it can be defined before you include the headers), and
implement `void serialEvent()` - which is part of the standard Arduino library
and gets called whenever there is incoming data on the serial line - and call
`MQTTSN::parse_stream()`

To use an RF12B module, you need the JeeNode library installed, and to define
`USE_RF12` on the command line. Then call `MQTTSN::parse_rf12()` as part of
your normal loop.

Subclass MQTTSN and override some, all, or none of the handler virtual
functions to get data from the broker or to change application state.

Most of the handler functions are stubs, but a few of them deal with return
code checking, topic registration and so on, so you should call the base class
implementation of the handler before any custom code.

Virtual functions seem to take significant space in the compiled binary, so if
you want to shave some bytes off your final build, comment out the handlers
you're not using in `mqttsn-messages.h` and in `void MQTTSN::dispatch()`

Message IDs are generated sequentially, and as per the spec, there is no
provision to queue up messages between acknowledgements; so if you call
REGISTER or PUBLISH with QoS1 or QoS2, you must wait for its corresponding
REGACK/PUBACK before issuing another message.

QoS1 is handled, and the library will wait for a relevant ACK with the
recommended delay and number of retries. If it still cannot get an ACK after
retrying, `MQTTSN::disconnect_handler()` is called (with a `NULL` pointer as
the message) which you should override and use to handle reconnection.

Due to it's relative complexity, QoS2 handshaking isn't handled (but the
message handlers are there, they need enabling by defining `USE_QOS2` on the
command line), and so it should be part of your own code if you want it.

Please file any bug reports in the issue tracker, or better yet fork and fix!

TODO
----

- Forwarding
- Buffer overrun checking
- Congestion handling
- More conditional compilation for different features
- Examples
- Tests

[MQTT-SN]:http://mqtt.org
[Uno]:http://arduino.cc/en/Main/arduinoBoardUno
[JeeNode]:http://jeelabs.net
