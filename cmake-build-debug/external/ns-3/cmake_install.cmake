# Install script for directory: /Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/external/ns-3

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-zigbee.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-wimax.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-wifi.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-virtual-net-device.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-uan.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-traffic-control.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-topology-read.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-stats.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-spectrum.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-sixlowpan.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-propagation.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-point-to-point-layout.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-point-to-point.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-olsr.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-nix-vector-routing.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-network.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-netanim.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-mobility.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-mesh.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-lte.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-lr-wpan.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-internet-apps.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-internet.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-flow-monitor.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-energy.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-dsr.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-dsdv.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-csma-layout.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-csma.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-core.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-config-store.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-buildings.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-bridge.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-applications.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-aodv.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/pkgconfig/ns3-antenna.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3/ns3Targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3/ns3Targets.cmake"
         "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/CMakeFiles/Export/2a81891a6eb0eac8fbb86ed7caf9a458/ns3Targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3/ns3Targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3/ns3Targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/CMakeFiles/Export/2a81891a6eb0eac8fbb86ed7caf9a458/ns3Targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3" TYPE FILE FILES "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/CMakeFiles/Export/2a81891a6eb0eac8fbb86ed7caf9a458/ns3Targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ns3" TYPE FILE FILES
    "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/ns3Config.cmake"
    "/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/ns3ConfigVersion.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/src/cmake_install.cmake")
  include("/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/examples/cmake_install.cmake")
  include("/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/scratch/cmake_install.cmake")
  include("/Users/noahljungberg/programmering/cpp/tdde35-project-noalj314-oscca863-kevma271-malbe283/cmake-build-debug/external/ns-3/utils/cmake_install.cmake")

endif()

