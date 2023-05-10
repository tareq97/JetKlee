# Install script for directory: /opt/symbiotic/klee/runtime/Runtest

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/opt/symbiotic/install/llvm-10.0.0/lib" TYPE SHARED_LIBRARY FILES "/opt/symbiotic/klee/build-10.0.0/lib/libkleeRuntest.so.1.0")
  if(EXISTS "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so.1.0")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/opt/symbiotic/install/llvm-10.0.0/lib" TYPE SHARED_LIBRARY FILES "/opt/symbiotic/klee/build-10.0.0/lib/libkleeRuntest.so")
  if(EXISTS "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/opt/symbiotic/install/llvm-10.0.0/lib/libkleeRuntest.so")
    endif()
  endif()
endif()

