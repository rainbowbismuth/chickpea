cmake_minimum_required(VERSION 3.16)
set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
project(chickpea_c C)

set(CMAKE_C_STANDARD 99)

set(DEVKITARM /opt/devkitpro/devkitARM/bin)
set(DEVKITPRO /opt/devkitpro/tools/bin)

set(CMAKE_C_LINK_EXECUTABLE "${DEVKITARM}/arm-none-eabi-ld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -nostdlib -target arm-none-eabi -mcpu=arm7tdmi -mthumb")
set(CMAKE_C_FLAGS_DEBUG "-O1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/src/linker.ld")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_custom_command(
        OUTPUT crt0.o
        DEPENDS ${CMAKE_SOURCE_DIR}/src/crt0.s
        COMMAND ${DEVKITARM}/arm-none-eabi-as ${CMAKE_SOURCE_DIR}/src/crt0.s -o crt0.o
)

set(SRC_FILES src/main.c)
set(INCLUDE_FILES include/chickpea.h include/chickpea/common.h include/chickpea/gba.h)

add_executable(chickpea_c.elf ${SRC_FILES} ${INCLUDE_FILES} crt0.o)

add_custom_target(
        chickpea_c.gba ALL
        DEPENDS chickpea_c.elf
        COMMAND ${DEVKITARM}/arm-none-eabi-objcopy -O binary chickpea_c.elf chickpea_c.gba
        COMMAND ${DEVKITPRO}/gbafix chickpea_c.gba
        BYPRODUCTS chickpea_c.gba
)
