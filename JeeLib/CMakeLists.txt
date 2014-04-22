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

set(SOURCES
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/JeeLib.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsBMP085.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsBMP085.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/Ports.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/Ports.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsLCD.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsLCD.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsRF12.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsSHT11.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/PortsSHT11.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/RF12.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/RF12.h
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/RF12sio.cpp
    ${ARDUINO_LIBRARIES_PATH}/JeeLib/RF12sio.h
)

add_avr_library(JeeLib ${SOURCES})
