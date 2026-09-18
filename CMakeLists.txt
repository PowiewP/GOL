cmake_minimum_required(VERSION 4.2)
project(GOL)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.16 # pin explicitly; bump when you want a newer SDL3
        GIT_SHALLOW TRUE
)

# Trim SDL3 to just what LsInput needs (gamepad input) before it configures.
# These MUST be CACHE entries with FORCE, set before FetchContent_MakeAvailable --
# SDL's own CMakeLists uses option()/dep_option(), which only sets a variable
# if it isn't already a cache entry, so a plain set() here would be ignored.
set(SDL_SHARED   OFF CACHE BOOL "" FORCE)
set(SDL_STATIC   ON  CACHE BOOL "" FORCE)
set(SDL_VIDEO    OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO    OFF CACHE BOOL "" FORCE)
set(SDL_RENDER   OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA   OFF CACHE BOOL "" FORCE)
set(SDL_GPU      OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR   OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK ON  CACHE BOOL "" FORCE) # required -- gamepad sits on top of this
set(SDL_HAPTIC   OFF  CACHE BOOL "" FORCE) # set OFF if you don't need rumble

FetchContent_MakeAvailable(SDL3)

add_library(GOL STATIC
        HeliosDac.cpp
        HeliosDac.h
        idn/idn-hello.h
        idn/idn-stream.h
        idn/idn.cpp
        idn/idn.h
        idn/idnServerList.cpp
        idn/idnServerList.h
        idn/plt-windows.cpp
        idn/plt-windows.h
        libusb.h

        LsRendering.h
        LsRendering.cpp
        LsUtilities.h
        LsUtilities.cpp
        LsInput.h
        LsInput.cpp)

target_include_directories(GOL PUBLIC include)

# PRIVATE keeps SDL3 headers/types out of LsInput.h's public surface (per the
# PImpl wrapper), but since GOL is a STATIC library, PRIVATE also means this
# link requirement does NOT propagate to anything that links against GOL --
# static libraries don't resolve symbols at their own build step, so any
# final executable linking GOL must link SDL3 again itself (see GOLTest below).
target_link_libraries(GOL PRIVATE SDL3::SDL3-static)

add_executable(GOLTest
        HeliosDac.cpp
        HeliosDac.h
        idn/idn-hello.h
        idn/idn-stream.h
        idn/idn.cpp
        idn/idn.h
        idn/idnServerList.cpp
        idn/idnServerList.h
        idn/plt-windows.cpp
        idn/plt-windows.h
        libusb.h

        testing.cpp
        LsRendering.h
        LsRendering.cpp
        LsUtilities.h
        LsUtilities.cpp)
target_link_libraries(GOLTest PRIVATE GOL
        SDL3::SDL3-static
        usb-1.0
        ws2_32
        winmm
        iphlpapi)