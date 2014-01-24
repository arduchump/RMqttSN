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

cmake_minimum_required(VERSION 2.6)

set(CMAKE_TOOLCHAIN_FILE avr-gcc.cmake)
set(CMAKE_SYSTEM_NAME Generic)

project(mqttsn-arduino)

include_directories(
    ${ARDUINO_LIBRARY_PATH}/cores/arduino
    ${ARDUINO_LIBRARY_PATH}/variants/standard
)

# Using the Arduino IDE libraries, we may need to define one or more of the
# following: -DUSB_VID=null -DUSB_PID=null

add_definitions(-DF_CPU=16000000L)
add_definitions(-MMD)
add_definitions(-DARDUINO=105)

set(MQTTSN_SOURCES
    src/mqttsn-messages.cpp
)

add_avr_library(mqttsn-arduino ${MQTTSN_SOURCES})
