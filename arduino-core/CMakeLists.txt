# CMakeLists.txt
# The MIT License (MIT)
# Copyright (C) 2014 John Donovan
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

set(CORE_LIB_SOURCES
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/wiring.c
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/HardwareSerial.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/Print.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/CDC.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/HID.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/IPAddress.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/new.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/Stream.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/Tone.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/USBCore.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/WMath.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/WString.cpp
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/WInterrupts.c
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/wiring_analog.c
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/wiring_digital.c
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/wiring_pulse.c
    ${ARDUINO_LIBRARY_PATH}/cores/arduino/wiring_shift.c
    #${ARDUINO_LIBRARY_PATH}/cores/arduino/avr-libc/malloc.c
    #${ARDUINO_LIBRARY_PATH}/cores/arduino/avr-libc/realloc.c
)

include_directories(
    ${ARDUINO_LIBRARY_PATH}/cores/arduino
    ${ARDUINO_LIBRARY_PATH}/variants/standard
)

add_avr_library(core ${CORE_LIB_SOURCES})
