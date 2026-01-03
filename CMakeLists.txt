cmake_minimum_required(VERSION 3.10)
project(olcGimmeHead)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(gimme-head)
target_sources(gimme-head
    PRIVATE
    src_cpp/gimme-head.cpp
)
