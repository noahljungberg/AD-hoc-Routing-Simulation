cmake_minimum_required(VERSION 3.13)
project(TDDE35Project)


#
# 1) Enable C++23 at the top level:
#
set(CMAKE_CXX_STANDARD 23)             # Or 20, 23, etc.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# 1) If your ns-3 is a standard release with its own CMakeLists,
#    you can add_subdirectory if you want ns-3 to be built along with your code.
add_subdirectory(external/ns-3)  # This picks up ns-3’s CMakeLists.txt

# 2) Otherwise, if you install ns-3 system-wide or to some prefix,
#    you might do "find_package(ns3 REQUIRED)".

# 3) Gather your source files:
set(TDDE35_SOURCES
    src/main.cpp
    src/ns3test.cc
    src/simulations/AbstractSimulation.cpp
    src/simulations/StaticSimulation.cpp
)

set(TDDE35_HEADERS
    include/Simulations/AbstractSimulation.hpp
    include/Simulations/StaticSimulation.hpp
    # ...others
)

# 4) Create an executable for your main runner
add_executable(tdde35-runner src/main.cpp)

# 5) Link the ns-3 modules you need
#    For example, if you need core, internet, dsdv, etc.
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

# 6) If you want the compiler to see your local headers easily:
target_include_directories(tdde35-runner
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)