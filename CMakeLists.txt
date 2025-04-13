cmake_minimum_required(VERSION 3.13)
project(adhoc_routing_simulation CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to your ns-3 installation
set(NS3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ns-3)

# Find the ns-3 installation
if(NOT EXISTS ${NS3_DIR}/build/ns3/ns3-config-version.cmake)
    message(FATAL_ERROR "ns-3 not properly built in ${NS3_DIR}/build. Please build ns-3 first.")
endif()

# Find the ns-3 libraries and includes
list(APPEND CMAKE_PREFIX_PATH ${NS3_DIR}/build/ns3)
find_package(ns3 REQUIRED)

# Add your executable
add_executable(adhoc_routing_simulation examples/main.cpp)

# Set the output directory for the executable
set_target_properties(adhoc_routing_simulation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against ns-3 libraries
target_link_libraries(adhoc_routing_simulation PUBLIC ${ns3-libs})

# Include ns-3 headers
target_include_directories(adhoc_routing_simulation PRIVATE ${ns3-include-dirs})

# Add a custom target to run the simulation
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/adhoc_routing_simulation
    DEPENDS adhoc_routing_simulation
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)