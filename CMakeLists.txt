cmake_minimum_required(VERSION 3.10)
project(chip8 C)

# Generate compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Set the build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

# Set the source files directory
set(SOURCES 
    src/main.c
    src/aux.c
    src/chip8.c
)

# Add an executable target
add_executable(chip8 ${SOURCES})

# Set compiler flags for debugging symbols
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")

# Set include directories for your target
target_include_directories(chip8 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link SDL2 libraries
target_link_libraries(chip8 ${SDL2_LIBRARIES})

