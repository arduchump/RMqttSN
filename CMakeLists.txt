# CMakeLists.txt
# Copyright (C) 2013 Housahedron
#
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
