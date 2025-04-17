cmake_minimum_required(VERSION 3.13)
project(TDDE35Project)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add ns-3 subdirectory
add_subdirectory(external/ns-3)

# Create an executable with ALL your source files
add_executable(tdde35-runner
    src/main.cpp
    src/simulations/AbstractSimulation.cpp
    src/simulations/StaticSimulation.cpp
            src/simulations/StaticSimulationGPSR.cpp
    src/gpsr/gpsr.cpp
    src/gpsr/gpsr-helper.cpp
    src/gpsr/gpsr-packet.cpp
    src/gpsr/gpsr-ptable.cpp
    src/gpsr/gpsr-rqueue.cpp
)

# Link with ns-3 modules
target_link_libraries(tdde35-runner
    PRIVATE
    ns3::core
    ns3::network
    ns3::internet
    ns3::applications
    ns3::mobility
    ns3::wifi
    ns3::dsdv
    ns3::dsr
    ns3::aodv
    ns3::olsr
    ns3::flow-monitor
    ns3::stats
)

# Include your header files
target_include_directories(tdde35-runner
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include

)