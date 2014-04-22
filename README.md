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

mqttsn-serial-bridge
--------------------
This program is used to interface a JeeNode with a serial port.

To use it, flash the "base" JeeNode with the code, and hook it up to a serial
port on your computer or RPi. You will notice a couple of CMakeLists.txt
files that I've added into JeeNode and arduino-core directories, the build
system should pick these up and compile and link the two libraries. The
easiest way I have found to then get it talking to an MQTT-SN broker is to
use socat:

    socat /dev/ttyUSB0,raw,echo=0,b115200 UDP4:127.0.0.1:1883

Which bridges between the serial port and a TCP/IP UDP port that your broker
is listening on. Naturally you may have to change the tty device to whichever
one you've plugged the JeeNode in to.

I've not tried this with a RPi yet, but it is on the list of things to do. The
only possible problem I can see is that the RPi's serial GPIO doesn't have a
DTR pin, but I think socat will cope. If not, I'll write a simple bit of
Python that emulates socat and handles DTR, but I'd rather not!

For my MQTT broker, I am using RSMB because it is the only one I've found that
handles MQTT-SN. I have a simple config file for it that listens for MQTT-SN
on port 1883 and MQTT on 1884, thereby acting as a bridge between the two
protocols:

    listener 1883 INADDR_ANY mqtts
    listener 1884 INADDR_ANY mqtt

And so from here I can subscribe with any regular MQTT client.

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
