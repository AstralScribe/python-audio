# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mayank/Documents/dev-hub/python-audio

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mayank/Documents/dev-hub/python-audio/cmake

# Include any dependencies generated for this target.
include CMakeFiles/_audioop.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/_audioop.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/_audioop.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/_audioop.dir/flags.make

CMakeFiles/_audioop.dir/lib/audioop.cpp.o: CMakeFiles/_audioop.dir/flags.make
CMakeFiles/_audioop.dir/lib/audioop.cpp.o: /home/mayank/Documents/dev-hub/python-audio/lib/audioop.cpp
CMakeFiles/_audioop.dir/lib/audioop.cpp.o: CMakeFiles/_audioop.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/mayank/Documents/dev-hub/python-audio/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/_audioop.dir/lib/audioop.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/_audioop.dir/lib/audioop.cpp.o -MF CMakeFiles/_audioop.dir/lib/audioop.cpp.o.d -o CMakeFiles/_audioop.dir/lib/audioop.cpp.o -c /home/mayank/Documents/dev-hub/python-audio/lib/audioop.cpp

CMakeFiles/_audioop.dir/lib/audioop.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/_audioop.dir/lib/audioop.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mayank/Documents/dev-hub/python-audio/lib/audioop.cpp > CMakeFiles/_audioop.dir/lib/audioop.cpp.i

CMakeFiles/_audioop.dir/lib/audioop.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/_audioop.dir/lib/audioop.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mayank/Documents/dev-hub/python-audio/lib/audioop.cpp -o CMakeFiles/_audioop.dir/lib/audioop.cpp.s

# Object files for target _audioop
_audioop_OBJECTS = \
"CMakeFiles/_audioop.dir/lib/audioop.cpp.o"

# External object files for target _audioop
_audioop_EXTERNAL_OBJECTS =

_audioop.cpython-311-x86_64-linux-gnu.so: CMakeFiles/_audioop.dir/lib/audioop.cpp.o
_audioop.cpython-311-x86_64-linux-gnu.so: CMakeFiles/_audioop.dir/build.make
_audioop.cpython-311-x86_64-linux-gnu.so: CMakeFiles/_audioop.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/mayank/Documents/dev-hub/python-audio/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared module _audioop.cpython-311-x86_64-linux-gnu.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/_audioop.dir/link.txt --verbose=$(VERBOSE)
	/usr/bin/strip /home/mayank/Documents/dev-hub/python-audio/cmake/_audioop.cpython-311-x86_64-linux-gnu.so

# Rule to build all files generated by this target.
CMakeFiles/_audioop.dir/build: _audioop.cpython-311-x86_64-linux-gnu.so
.PHONY : CMakeFiles/_audioop.dir/build

CMakeFiles/_audioop.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/_audioop.dir/cmake_clean.cmake
.PHONY : CMakeFiles/_audioop.dir/clean

CMakeFiles/_audioop.dir/depend:
	cd /home/mayank/Documents/dev-hub/python-audio/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mayank/Documents/dev-hub/python-audio /home/mayank/Documents/dev-hub/python-audio /home/mayank/Documents/dev-hub/python-audio/cmake /home/mayank/Documents/dev-hub/python-audio/cmake /home/mayank/Documents/dev-hub/python-audio/cmake/CMakeFiles/_audioop.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/_audioop.dir/depend
