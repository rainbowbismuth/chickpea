cmake_minimum_required(VERSION 3.16)
set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
project(chickpea_c C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_DEBUG "-O1")

include_directories(${CMAKE_SOURCE_DIR}/include)
set(SRC_FILES src/main.c)
set(INCLUDE_FILES include/chickpea.h include/chickpea/common.h include/chickpea/platform.h)

if (BUILD_GBA)

    set(DEVKITARM /opt/devkitpro/devkitARM/bin)
    set(DEVKITPRO /opt/devkitpro/tools/bin)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_C_LINK_EXECUTABLE "${DEVKITARM}/arm-none-eabi-ld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/src/linker.ld")

    set(SRC_FILES ${SRC_FILES} src/gba.c)
    set(INCLUDE_FILES ${INCLUDE_FILES} include/chickpea/gba.h)

    add_custom_command(
            OUTPUT crt0.o
            DEPENDS ${CMAKE_SOURCE_DIR}/src/crt0.s
            COMMAND ${DEVKITARM}/arm-none-eabi-as ${CMAKE_SOURCE_DIR}/src/crt0.s -o crt0.o
    )
    add_executable(chickpea_c.elf ${SRC_FILES} ${INCLUDE_FILES} crt0.o)
    set_target_properties(chickpea_c.elf
            PROPERTIES
            COMPILE_FLAGS "-Wall -Wextra -nostdlib -target arm-none-eabi -mcpu=arm7tdmi -mthumb"
            COMPILE_DEFINITIONS BUILD_GBA=1
            )

    add_custom_target(
            chickpea_c.gba ALL
            DEPENDS chickpea_c.elf
            COMMAND ${DEVKITARM}/arm-none-eabi-objcopy -O binary chickpea_c.elf chickpea_c.gba
            COMMAND ${DEVKITPRO}/gbafix chickpea_c.gba
            BYPRODUCTS chickpea_c.gba
    )

else ()
    set(SRC_FILES ${SRC_FILES} src/emulated.c)
    set(INCLUDE_FILES ${INCLUDE_FILES} include/chickpea/emulated.h)
    add_executable(chickpea_c ${SRC_FILES} ${INCLUDE_FILES})

endif ()






