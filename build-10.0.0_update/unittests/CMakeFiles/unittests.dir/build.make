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

# Utility rule file for unittests.

# Include the progress variables for this target.
include unittests/CMakeFiles/unittests.dir/progress.make

unittests/CMakeFiles/unittests: unittests/AssignmentTest
unittests/CMakeFiles/unittests: unittests/ExprTest
unittests/CMakeFiles/unittests: unittests/RefTest
unittests/CMakeFiles/unittests: unittests/SolverTest
unittests/CMakeFiles/unittests: unittests/TreeStreamTest
unittests/CMakeFiles/unittests: unittests/DiscretePDFTest
unittests/CMakeFiles/unittests: unittests/TimeTest
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/opt/symbiotic/klee/build-10.0.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Running unittests"
	cd /opt/symbiotic/klee/build-10.0.1/unittests && /opt/symbiotic/llvm-10.0.1/build/bin/llvm-lit -v -s /opt/symbiotic/klee/build-10.0.1/unittests

unittests: unittests/CMakeFiles/unittests
unittests: unittests/CMakeFiles/unittests.dir/build.make

.PHONY : unittests

# Rule to build all files generated by this target.
unittests/CMakeFiles/unittests.dir/build: unittests

.PHONY : unittests/CMakeFiles/unittests.dir/build

unittests/CMakeFiles/unittests.dir/clean:
	cd /opt/symbiotic/klee/build-10.0.1/unittests && $(CMAKE_COMMAND) -P CMakeFiles/unittests.dir/cmake_clean.cmake
.PHONY : unittests/CMakeFiles/unittests.dir/clean

unittests/CMakeFiles/unittests.dir/depend:
	cd /opt/symbiotic/klee/build-10.0.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/symbiotic/klee /opt/symbiotic/klee/unittests /opt/symbiotic/klee/build-10.0.1 /opt/symbiotic/klee/build-10.0.1/unittests /opt/symbiotic/klee/build-10.0.1/unittests/CMakeFiles/unittests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : unittests/CMakeFiles/unittests.dir/depend

