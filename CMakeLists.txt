cmake_minimum_required(VERSION 3.10)

project(sasSerial
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set( SERIALPORT
    src/uartdriver.cpp src/uartdriver.h
    src/ringbuffer.cpp src/ringbuffer.h
    src/serialportsas.cpp src/serialportsas.h
)

add_executable(sasSerial src/main.cpp ${SERIALPORT})
