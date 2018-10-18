cmake_minimum_required(VERSION 3.12)
project(SISL)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
include_directories(headers)

add_executable(SISL
        headers/CallStrategies.hpp
        headers/Delegate.hpp
        headers/Generic_Delegate.hpp
        headers/Signal.hpp
        headers/Object.hpp
        headers/SlotsManager.hpp
        headers/Utils.h
        Sisl.hpp main.cpp)
