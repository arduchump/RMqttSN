set(SOURCES
    JeeLib.h
    PortsBMP085.cpp
    PortsBMP085.h
    Ports.cpp
    Ports.h
    PortsLCD.cpp
    PortsLCD.h
    PortsRF12.cpp
    PortsSHT11.cpp
    PortsSHT11.h
    RF12.cpp
    RF12.h
    RF12sio.cpp
    RF12sio.h
)

add_avr_library(JeeLib ${SOURCES})