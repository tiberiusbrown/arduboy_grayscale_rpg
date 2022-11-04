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
add_executable(arduboy_grayscale_rpg
    .editorconfig
    src/battle.cpp
    src/common.hpp
    src/common.cpp
    src/draw.cpp
    src/font_adv.hpp
    src/init.cpp
    src/main_sdl.cpp
    src/map.cpp
    src/platform.cpp
    src/render.cpp
    src/script_commands.hpp
    src/tile_solid.hpp
    src/update.cpp
    )
target_link_libraries(arduboy_grayscale_rpg SDL2-static SDL2main)
