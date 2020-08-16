cmake_minimum_required(VERSION 3.16)
set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
project(chickpea C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SRC_FILES src/main.c src/chickpea/common.c src/game/debug_font.c
        src/test_suites.c src/chickpea/nano_unit.c src/chickpea/bit_vec.c
        src/game/map.c src/game/sprite.c src/game/object_tiles.c
        src/game/input.c src/game/random.c src/game/font.c src/game/text_box.c
        baked/baked.c src/game/resource.c)
set(INCLUDE_FILES include/chickpea.h include/chickpea/common.h
        include/chickpea/platform.h include/game/debug_font.h
        include/chickpea/nano_unit.h include/chickpea/bit_vec.h
        include/game/map.h include/chickpea/vec2.h include/game/sprite.h
        include/game/object_tiles.h include/game/input.h include/game/random.h
        include/game/screen.h include/game/font.h include/game/text_box.h
        include/game/resource.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wstrict-aliasing")

if (BUILD_GBA)
    set(DEVKITARM /opt/devkitpro/devkitARM/bin)
    set(DEVKITARM_LIB /opt/devkitpro/devkitARM/lib/gcc/arm-none-eabi/10.1.0/)
    set(DEVKITPRO /opt/devkitpro/tools/bin)
    set(CHICKPEA_AS ${DEVKITARM}/arm-none-eabi-as)
endif ()

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
            COMMAND ${CHICKPEA_AS} ${CMAKE_SOURCE_DIR}/src/chickpea/crt0.s
            -o crt0.o
    )
    add_executable(chickpea.elf ${SRC_FILES} ${INCLUDE_FILES} crt0.o)
    set_target_properties(chickpea.elf
            PROPERTIES
            COMPILE_FLAGS "-Wall -Wextra -nostdlib -target arm-none-eabi\
                -mcpu=arm7tdmi -mthumb -ffreestanding"
            COMPILE_DEFINITIONS BUILD_GBA=1
            )

    add_custom_target(
            chickpea.gba ALL
            DEPENDS chickpea.elf
            COMMAND ${DEVKITARM}/arm-none-eabi-objcopy -O binary chickpea.elf
            chickpea.gba
            COMMAND ${DEVKITPRO}/gbafix chickpea.gba
            BYPRODUCTS chickpea.gba
    )
else ()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address \
        -fsanitize=undefined")

    set(SRC_FILES ${SRC_FILES} src/chickpea/emulated.c)
    set(INCLUDE_FILES ${INCLUDE_FILES} include/chickpea/emulated.h)

    if (CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
        string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
        add_executable("${BUILD_DATE}_chickpea" ${SRC_FILES} ${INCLUDE_FILES})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s FILESYSTEM=0 -flto=thin")
    else ()
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})

        add_executable(chickpea ${SRC_FILES} ${INCLUDE_FILES})
        target_link_libraries(chickpea ${SDL2_LIBRARIES})
    endif ()
endif ()
