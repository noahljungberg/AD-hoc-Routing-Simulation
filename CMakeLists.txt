cmake_minimum_required(VERSION 3.10)
project(GPSRProject)

# C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# NS-3 path in the Docker container
set(NS3_DIR "/ns-3")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/simulations
    ${NS3_DIR}/build/include
)

# Find all source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/simulations/*.cpp"
)

# Create executable
add_executable(GPSR ${SOURCES})

# Link with NS-3 libraries
target_link_directories(GPSR PRIVATE ${NS3_DIR}/build/lib)

# List of NS-3 libraries needed
target_link_libraries(GPSR PRIVATE
    ns3-core
    ns3-network
    ns3-internet
    ns3-applications
    ns3-mobility
    ns3-wifi
    ns3-dsdv
    ns3-dsr
    ns3-aodv
    ns3-olsr
    ns3-flow-monitor
    ns3-stats
)