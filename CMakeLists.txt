cmake_minimum_required(VERSION 3.10)

project(sasSerial
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(sasSerial src/main.cpp)
