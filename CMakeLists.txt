cmake_minimum_required(VERSION 3.16)
set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
project(chickpea_c C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR}/include)
set(SRC_FILES src/main.c src/chickpea/common.c src/game/debug_font.c src/test_suites.c
        src/chickpea/nano_unit.c src/chickpea/bit_vec.c src/game/map.c)
set(INCLUDE_FILES include/chickpea.h include/chickpea/common.h
        include/chickpea/platform.h include/game/debug_font.h
        include/chickpea/nano_unit.h include/chickpea/bit_vec.h include/game/map.h include/chickpea/vec2.h)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wstrict-aliasing")

if (BUILD_GBA)
    set(DEVKITARM /opt/devkitpro/devkitARM/bin)
    set(DEVKITARM_LIB /opt/devkitpro/devkitARM/lib/gcc/arm-none-eabi/10.1.0/)
    set(DEVKITPRO /opt/devkitpro/tools/bin)
    set(CHICKPEA_AS ${DEVKITARM}/arm-none-eabi-as)
else ()
    set(CHICKPEA_AS as)
endif ()

add_custom_command(
        OUTPUT embedded_data.o
        DEPENDS ${CMAKE_SOURCE_DIR}/src/game/embedded_data.s
        BYPRODUCTS embedded_data.o
        COMMAND ${CHICKPEA_AS} ${CMAKE_SOURCE_DIR}/src/game/embedded_data.s
            -o embedded_data.o
)

if (BUILD_GBA)
    set(CMAKE_C_LINK_EXECUTABLE "${DEVKITARM}/arm-none-eabi-ld <LINK_FLAGS>\
        <OBJECTS> -o <TARGET> -L${DEVKITARM_LIB} -lgcc <LINK_LIBRARIES>")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
        -T${CMAKE_SOURCE_DIR}/src/chickpea/linker.ld")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1")

    set(SRC_FILES ${SRC_FILES} src/chickpea/gba.c)
    set(INCLUDE_FILES ${INCLUDE_FILES} include/chickpea/gba.h)

    add_custom_command(
            OUTPUT crt0.o
            DEPENDS ${CMAKE_SOURCE_DIR}/src/chickpea/crt0.s
            BYPRODUCTS crt0.o
            COMMAND ${CHICKPEA_AS} ${CMAKE_SOURCE_DIR}/src/crt0.s -o crt0.o
    )
    add_executable(chickpea_c.elf ${SRC_FILES} ${INCLUDE_FILES} crt0.o
            embedded_data.o)
    set_target_properties(chickpea_c.elf
            PROPERTIES
            COMPILE_FLAGS "-Wall -Wextra -nostdlib -target arm-none-eabi\
                -mcpu=arm7tdmi -mthumb"
            COMPILE_DEFINITIONS BUILD_GBA=1
            )

    add_custom_target(
            chickpea_c.gba ALL
            DEPENDS chickpea_c.elf
            COMMAND ${DEVKITARM}/arm-none-eabi-objcopy -O binary chickpea_c.elf
                chickpea_c.gba
            COMMAND ${DEVKITPRO}/gbafix chickpea_c.gba
            BYPRODUCTS chickpea_c.gba
    )
else ()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -fsanitize=address \
        -fsanitize=undefined")

    set(SRC_FILES ${SRC_FILES} src/chickpea/emulated.c)
    set(INCLUDE_FILES ${INCLUDE_FILES} include/chickpea/emulated.h)
    add_executable(chickpea_c ${SRC_FILES} ${INCLUDE_FILES} embedded_data.o)
    set_target_properties(chickpea_c
            PROPERTIES
            COMPILE_FLAGS "-I/usr/local/include/SDL2 -D_THREAD_SAFE"
            LINK_FLAGS "-L/usr/local/lib -lSDL2")
endif ()






