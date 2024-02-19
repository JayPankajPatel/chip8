cmake_minimum_required(VERSION 3.10)
project(chip8 C)

# Find SDL2 package
find_package(SDL2 REQUIRED)
# Set the build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

# Set the source files directory
set(SOURCES src/main.c)

# Add an executable target
add_executable(chip8 ${SOURCES})

# Link SDL2 libraries
target_link_libraries(chip8 ${SDL2_LIBRARIES})
# Set compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# Generate compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

