cmake_minimum_required(VERSION 4.2)
project(GOL)

set(CMAKE_CXX_STANDARD 20)

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
        LsUtilities.cpp)

target_include_directories(GOL PUBLIC include)

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
        usb-1.0
        ws2_32
        winmm
        iphlpapi)
