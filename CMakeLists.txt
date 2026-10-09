cmake_minimum_required(VERSION 4.2)
project(GOL)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL3 REQUIRED CONFIG)

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
        LsInput.cpp
        ILDAFormat.cpp
        BuildOptimalOutput.cpp)

target_include_directories(GOL PUBLIC include)

# PRIVATE keeps SDL3 out of LsInput.h's public surface, but since GOL is a
# STATIC library the link requirement does not propagate, so GOLTest must
# link SDL3 itself.
target_link_libraries(GOL PRIVATE SDL3::SDL3)

add_executable(GOLTest testing.cpp)
target_link_libraries(GOLTest PRIVATE GOL SDL3::SDL3 usb-1.0 ws2_32 winmm iphlpapi)