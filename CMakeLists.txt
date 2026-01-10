cmake_minimum_required(VERSION 3.10)
project(olcGimmeHead)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# explicitly set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(gimme-head)
target_sources(gimme-head
    PRIVATE
    src_cpp/gimme-head.cpp
)

# force Visual Studio to use the binary directory without creating Debug/Release directories
if(MSVC)
    set_target_properties(gimme-head PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY   "$<1:${CMAKE_BINARY_DIR}>"
        LIBRARY_OUTPUT_DIRECTORY   "$<1:${CMAKE_BINARY_DIR}>"
        ARCHIVE_OUTPUT_DIRECTORY   "$<1:${CMAKE_BINARY_DIR}>"
    )
endif()

# Allow emscripten access to the host filesystem
if(EMSCRIPTEN)
    target_link_options(gimme-head
        PRIVATE
        -sNODERAWFS
    )
endif()
