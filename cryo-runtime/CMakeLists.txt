cmake_minimum_required(VERSION 4.1)

project(cryo)

add_executable(cryo src/main.cpp
            src/cryopch.h
            src/cryopch.cpp)

include_directories(src)

target_precompile_headers(cryo
    PUBLIC
        src/cryopch.h
)
