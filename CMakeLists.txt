cmake_minimum_required(VERSION 3.10)

# Name of your project
project(GPSRProject)

# C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for header files
include_directories(
    include/network
    include/protocols
)

# Source files
file(GLOB_RECURSE SOURCES "src/**/*.cpp")

# Define the executable
add_executable(GPSR ${SOURCES})