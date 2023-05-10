# Install script for directory: /opt/symbiotic/klee/tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/opt/symbiotic/install/llvm-10.0.0")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/opt/symbiotic/klee/build-10.0.0/tools/gen-bout/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/gen-random-bout/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/kleaver/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/klee/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/klee-replay/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/klee-stats/cmake_install.cmake")
  include("/opt/symbiotic/klee/build-10.0.0/tools/ktest-tool/cmake_install.cmake")

endif()

