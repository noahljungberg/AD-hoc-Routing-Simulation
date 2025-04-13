cmake_minimum_required(VERSION 3.10)

# Name of your project
project(GPSRProject)

# C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include headers from the "include" folder
include_directories(include)

# Source files (adjust if needed)
file(GLOB SOURCES "src/*.cpp")

# Define the executable
add_executable(GPSR ${SOURCES})
