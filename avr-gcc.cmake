# avr-gcc.cmake
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

# Modified from https://github.com/agnat/powerbar

find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_SIZE_TOOL avr-size)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER   ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})

# XXX better use a seperate file for non toolchain stuff?
find_program(AVRDUDE avrdude)

add_definitions( -Wall )

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(
    AVR_PROGRAMMER arduino
    CACHE STRING
    "programmer hardware model. See 'avrdude -c ?' for a full list."
)

set(
    AVRDUDE_OPTIONS
    CACHE STRING
    "additional avrdude options"
)

set(
    AVR_DEFAULT_MCUS atmega328p
    CACHE STRING
    "List of default target MCUs. See 'avr-gcc --target-help' for valid values."
)

set(
    ARDUINO_LIBRARY_PATH
    CACHE STRING
    "Path to the standard Arduino libraries."
)

function(_avr_get_mcu_list NAME OUTVAR)
    string(TOUPPER ${NAME} name_uc)
    set(mcu_type_var ${name_uc}_MCUS)
    set(
        ${mcu_type_var} ${AVR_DEFAULT_MCUS}
        CACHE STRING
        "List of target MCUs. See 'avr-gcc --target-help' for valid values."
    )
    set(${OUTVAR} ${${mcu_type_var}} PARENT_SCOPE)
endfunction(_avr_get_mcu_list)

function(add_avr_executable EXECUTABLE_NAME)
    _avr_get_mcu_list(${EXECUTABLE_NAME} target_mcus)
    foreach(mcu ${target_mcus})
        set(elf_file ${EXECUTABLE_NAME}-${mcu}.elf)
        set(hex_file ${EXECUTABLE_NAME}-${mcu}.hex)
        set(map_file ${EXECUTABLE_NAME}-${mcu}.map)
        add_executable(${elf_file} EXCLUDE_FROM_ALL ${ARGN})
        set(common_opts "-mmcu=${mcu} -fshort-enums -fno-exceptions -ffunction-sections -fdata-sections")
        set_target_properties(
            ${elf_file}
            PROPERTIES
                COMPILE_FLAGS "${common_opts}"
                LINK_FLAGS "${common_opts} -Wl,--gc-sections,-Map,${map_file}"
        )

        add_custom_command(
            OUTPUT ${hex_file}
            COMMAND
                ${AVR_OBJCOPY} -j .text -j .data -O ihex ${elf_file} ${hex_file}
            COMMAND
                ${AVR_SIZE_TOOL} ${elf_file}
#            COMMAND
#                ${CMAKE_COMMAND} -E copy ${hex_file} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            DEPENDS ${elf_file}
        )

        list(APPEND all_hex_files ${hex_file})
        list(APPEND all_map_files ${map_file})
    endforeach(mcu ${target_mcus})
    set(eeprom_image ${EXECUTABLE_NAME}-eeprom.hex)
    add_custom_command(
        OUTPUT ${eeprom_image}
        COMMAND
            ${AVR_OBJCOPY} -j .eeprom --change-section-lma .eeprom=0
                -O ihex ${elf_file} ${eeprom_image}
        DEPENDS ${elf_file}
    )
    list(APPEND all_hex_files ${eeprom_image})
    add_custom_target(
        ${EXECUTABLE_NAME}
        ALL
        DEPENDS ${all_hex_files}
    )
    get_directory_property(clean_files ADDITIONAL_MAKE_CLEAN_FILES)
    list(APPEND clean_files ${all_map_files})
    set_directory_properties(
        PROPERTIES
            ADDITIONAL_MAKE_CLEAN_FILES "${clean_files}"
    )

    string(TOUPPER ${EXECUTABLE_NAME} name_uc)
    list(GET target_mcus 0 default_upload_mcu)
    set(
        ${name_uc}_UPLOAD_MCU ${default_upload_mcu}
        CACHE STRING
        "The MCU this executable will be uploaded to."
    )
    set(
        TTY_PORT /dev/ttyUSB0
        CACHE STRING
        "The tty to upload to."
    )
    set(
        TTY_BAUD 115200
        CACHE NUMBER
        "The Baud rate for uploading to the MCU."
    )
    set(upload_mcu ${${name_uc}_UPLOAD_MCU})
    set(upload_file ${EXECUTABLE_NAME}-${upload_mcu}.hex)
    add_custom_target(
        upload_${EXECUTABLE_NAME}
        ${AVRDUDE} -p ${upload_mcu} -c ${AVR_PROGRAMMER} ${AVRDUDE_OPTIONS}
            -U flash:w:${upload_file}
            -U eeprom:w:${eeprom_image}
            -P ${USB} -b ${TTY_BAUD}
        DEPENDS ${upload_file} ${eeprom_image}
        COMMENT "Uploading ${upload_file} to ${upload_mcu} using programmer ${AVR_PROGRAMMER}"
    )
    add_custom_target(
        disassemble_${EXECUTABLE_NAME}
        avr-objdump -h -S ${EXECUTABLE_NAME}-${upload_mcu}.elf > ${EXECUTABLE_NAME}.lst
        DEPENDS ${EXECUTABLE_NAME}-${upload_mcu}.elf
    )
endfunction(add_avr_executable)

function(add_avr_library LIBRARY_NAME)
    _avr_get_mcu_list(${LIBRARY_NAME} target_mcus)
    foreach(mcu ${target_mcus})
        set(lib_file ${LIBRARY_NAME}-${mcu})
        add_library(${lib_file} STATIC ${ARGN})
        set(common_opts "-mmcu=${mcu} -fshort-enums -fno-exceptions -ffunction-sections -fdata-sections")
        set_target_properties(
            ${lib_file}
            PROPERTIES
                COMPILE_FLAGS "${common_opts}"
                LINK_FLAGS "${common_opts}"# -Wl,-Map,${map_file}"
        )
    endforeach(mcu ${target_mcus})
endfunction(add_avr_library)

function(target_link_avr_libraries EXECUTABLE_NAME)
    _avr_get_mcu_list(${EXECUTABLE_NAME} target_mcus)

    foreach(mcu ${target_mcus})
        set(elf_file ${EXECUTABLE_NAME}-${mcu}.elf)
        foreach (lib_name ${ARGN})
            set(lib_file ${lib_name}-${mcu})
            target_link_libraries(${elf_file} ${lib_file})
        endforeach (lib_name ${ARGN})
    endforeach(mcu ${target_mcus})

endfunction(target_link_avr_libraries)
