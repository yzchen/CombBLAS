# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/cheny0l/work/db245/CombBLAS_beta_16_1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cheny0l/work/db245/CombBLAS_beta_16_1/build

# Include any dependencies generated for this target.
include ReleaseTests/CMakeFiles/TransposeTest.dir/depend.make

# Include the progress variables for this target.
include ReleaseTests/CMakeFiles/TransposeTest.dir/progress.make

# Include the compile flags for this target's objects.
include ReleaseTests/CMakeFiles/TransposeTest.dir/flags.make

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o: ReleaseTests/CMakeFiles/TransposeTest.dir/flags.make
ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o: ../ReleaseTests/TransposeTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cheny0l/work/db245/CombBLAS_beta_16_1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o"
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests && /usr/bin/mpicxx   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/TransposeTest.dir/TransposeTest.o -c /home/cheny0l/work/db245/CombBLAS_beta_16_1/ReleaseTests/TransposeTest.cpp

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TransposeTest.dir/TransposeTest.i"
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cheny0l/work/db245/CombBLAS_beta_16_1/ReleaseTests/TransposeTest.cpp > CMakeFiles/TransposeTest.dir/TransposeTest.i

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TransposeTest.dir/TransposeTest.s"
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cheny0l/work/db245/CombBLAS_beta_16_1/ReleaseTests/TransposeTest.cpp -o CMakeFiles/TransposeTest.dir/TransposeTest.s

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.requires:

.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.requires

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.provides: ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.requires
	$(MAKE) -f ReleaseTests/CMakeFiles/TransposeTest.dir/build.make ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.provides.build
.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.provides

ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.provides.build: ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o


# Object files for target TransposeTest
TransposeTest_OBJECTS = \
"CMakeFiles/TransposeTest.dir/TransposeTest.o"

# External object files for target TransposeTest
TransposeTest_EXTERNAL_OBJECTS =

ReleaseTests/TransposeTest: ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o
ReleaseTests/TransposeTest: ReleaseTests/CMakeFiles/TransposeTest.dir/build.make
ReleaseTests/TransposeTest: libCommGridlib.a
ReleaseTests/TransposeTest: libMPITypelib.a
ReleaseTests/TransposeTest: libMemoryPoollib.a
ReleaseTests/TransposeTest: libHashlib.a
ReleaseTests/TransposeTest: ReleaseTests/CMakeFiles/TransposeTest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cheny0l/work/db245/CombBLAS_beta_16_1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable TransposeTest"
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TransposeTest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
ReleaseTests/CMakeFiles/TransposeTest.dir/build: ReleaseTests/TransposeTest

.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/build

ReleaseTests/CMakeFiles/TransposeTest.dir/requires: ReleaseTests/CMakeFiles/TransposeTest.dir/TransposeTest.o.requires

.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/requires

ReleaseTests/CMakeFiles/TransposeTest.dir/clean:
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests && $(CMAKE_COMMAND) -P CMakeFiles/TransposeTest.dir/cmake_clean.cmake
.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/clean

ReleaseTests/CMakeFiles/TransposeTest.dir/depend:
	cd /home/cheny0l/work/db245/CombBLAS_beta_16_1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cheny0l/work/db245/CombBLAS_beta_16_1 /home/cheny0l/work/db245/CombBLAS_beta_16_1/ReleaseTests /home/cheny0l/work/db245/CombBLAS_beta_16_1/build /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests /home/cheny0l/work/db245/CombBLAS_beta_16_1/build/ReleaseTests/CMakeFiles/TransposeTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : ReleaseTests/CMakeFiles/TransposeTest.dir/depend
