# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /opt/symbiotic/klee

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /opt/symbiotic/klee/build-10.0.1

# Include any dependencies generated for this target.
include unittests/Ref/CMakeFiles/RefTest.dir/depend.make

# Include the progress variables for this target.
include unittests/Ref/CMakeFiles/RefTest.dir/progress.make

# Include the compile flags for this target's objects.
include unittests/Ref/CMakeFiles/RefTest.dir/flags.make

unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.o: unittests/Ref/CMakeFiles/RefTest.dir/flags.make
unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.o: ../unittests/Ref/RefTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/opt/symbiotic/klee/build-10.0.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.o"
	cd /opt/symbiotic/klee/build-10.0.1/unittests/Ref && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RefTest.dir/RefTest.cpp.o -c /opt/symbiotic/klee/unittests/Ref/RefTest.cpp

unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RefTest.dir/RefTest.cpp.i"
	cd /opt/symbiotic/klee/build-10.0.1/unittests/Ref && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /opt/symbiotic/klee/unittests/Ref/RefTest.cpp > CMakeFiles/RefTest.dir/RefTest.cpp.i

unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RefTest.dir/RefTest.cpp.s"
	cd /opt/symbiotic/klee/build-10.0.1/unittests/Ref && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /opt/symbiotic/klee/unittests/Ref/RefTest.cpp -o CMakeFiles/RefTest.dir/RefTest.cpp.s

# Object files for target RefTest
RefTest_OBJECTS = \
"CMakeFiles/RefTest.dir/RefTest.cpp.o"

# External object files for target RefTest
RefTest_EXTERNAL_OBJECTS =

unittests/RefTest: unittests/Ref/CMakeFiles/RefTest.dir/RefTest.cpp.o
unittests/RefTest: unittests/Ref/CMakeFiles/RefTest.dir/build.make
unittests/RefTest: lib/libgtest_main.a
unittests/RefTest: lib/libkleaverExpr.a
unittests/RefTest: lib/libgtest.a
unittests/RefTest: /usr/lib/x86_64-linux-gnu/libz.so
unittests/RefTest: /opt/symbiotic/llvm-10.0.1/build/lib/libLLVMSupport.a
unittests/RefTest: /opt/symbiotic/llvm-10.0.1/build/lib/libLLVMDemangle.a
unittests/RefTest: unittests/Ref/CMakeFiles/RefTest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/opt/symbiotic/klee/build-10.0.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../RefTest"
	cd /opt/symbiotic/klee/build-10.0.1/unittests/Ref && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RefTest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
unittests/Ref/CMakeFiles/RefTest.dir/build: unittests/RefTest

.PHONY : unittests/Ref/CMakeFiles/RefTest.dir/build

unittests/Ref/CMakeFiles/RefTest.dir/clean:
	cd /opt/symbiotic/klee/build-10.0.1/unittests/Ref && $(CMAKE_COMMAND) -P CMakeFiles/RefTest.dir/cmake_clean.cmake
.PHONY : unittests/Ref/CMakeFiles/RefTest.dir/clean

unittests/Ref/CMakeFiles/RefTest.dir/depend:
	cd /opt/symbiotic/klee/build-10.0.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/symbiotic/klee /opt/symbiotic/klee/unittests/Ref /opt/symbiotic/klee/build-10.0.1 /opt/symbiotic/klee/build-10.0.1/unittests/Ref /opt/symbiotic/klee/build-10.0.1/unittests/Ref/CMakeFiles/RefTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : unittests/Ref/CMakeFiles/RefTest.dir/depend

