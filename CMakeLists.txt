cmake_minimum_required(VERSION 2.8)

project(acidwarp C)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}")

set(CORE_SOURCES src/acidwarp.c src/palinit.c src/rolnfade.c src/display.c src/handy.c)
set(IMGGEN_SOURCES src/bit_map.c src/lut.c src/img_int.c src/img_float.c)
set(SOURCES ${CORE_SOURCES} ${IMGGEN_SOURCES} src/draw.c inc/acidwarp.h inc/bit_map.h inc/display.h inc/handy.h inc/lut.h inc/palinit.h inc/rolnfade.h inc/worker.h)

# Windows icon, built by converting PNG to ICO
# and then embedding using resource file.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_program(ICOTOOL icotool)
    if(ICOTOOL)
        add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/acidwarp.ico
                           COMMAND icotool -c -o ${CMAKE_SOURCE_DIR}/acidwarp.ico ${CMAKE_SOURCE_DIR}/acidwarp.png
                           MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/acidwarp.png)
        set(SOURCES ${SOURCES} acid_res.rc)
        set_source_files_properties(acid_res.rc PROPERTIES OBJECT_DEPENDS ${CMAKE_SOURCE_DIR}/acidwarp.ico)
    else(ICOTOOL)
        message(WARNING "Not adding icon to executable because icotool not found.")
    endif(ICOTOOL)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Icon set via SDL
find_program(XXD xxd)
find_program(CONVERT convert)

if(XXD AND CONVERT)
    add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/acidwarp.rgb
                       COMMAND convert ${CMAKE_SOURCE_DIR}/acidwarp.png -sample 64x64 ${CMAKE_SOURCE_DIR}/acidwarp.rgb
                       MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/acidwarp.png)
    add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/acid_ico.c
                       COMMAND xxd -i acidwarp.rgb > acid_ico.c
                       MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/acidwarp.rgb
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_definitions(-DADDICON)
    set(SOURCES ${SOURCES} acid_ico.c)
else(XXD AND CONVERT)
    if(NOT CONVERT)
        message(WARNING "ImageMagick convert is required resize icon for embedding")
    endif(NOT CONVERT)
    if(NOT XXD)
        message(WARNING "xxd is required to embed icon into code")
    endif(NOT XXD)
endif(XXD AND CONVERT)

include_directories("SDL/include")

add_executable(acidwarp ${SOURCES})

target_link_directories(acidwarp PUBLIC "SDL/lib/x64")
target_link_libraries(acidwarp PUBLIC "SDL2" "SDL2main")
target_include_directories(acidwarp PUBLIC "SDL/include")