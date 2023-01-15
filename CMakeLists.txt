cmake_minimum_required(VERSION 3.16)

if(MSVC)
    foreach(flag
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
        CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
        string(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
        set("${flag}" "${${flag}} /EHsc")
    endforeach()
endif()

include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        a1d1946dcba6509f0679f507b57e7b228d32e6f8 # 2.24.1
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(SDL2)

project(arduboy_grayscale_rpg)
set(EXEFLAG)
if(MSVC)
    set(EXEFLAG WIN32)
endif()
add_executable(arduboy_grayscale_rpg ${EXEFLAG}
    .editorconfig
    src/audio_sdl2.cpp
    src/battery.cpp
    src/battle.cpp
    src/common.hpp
    src/common.cpp
    src/draw.cpp
    src/gifenc.h
    src/gifenc.c
    src/init.cpp
    src/items.cpp
    src/main_sdl.cpp
    src/map.cpp
    src/pause.cpp
    src/pause_party.cpp
    src/platform.cpp
    src/render.cpp
    src/save.cpp
    src/script_commands.hpp
    src/update.cpp
    )
target_link_libraries(arduboy_grayscale_rpg SDL2-static SDL2main)
