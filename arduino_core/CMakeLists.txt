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
)

include_directories(
    ${ARDUINO_LIBRARY_PATH}/cores/arduino
    ${ARDUINO_LIBRARY_PATH}/variants/standard
)

add_avr_library(core ${CORE_LIB_SOURCES})
