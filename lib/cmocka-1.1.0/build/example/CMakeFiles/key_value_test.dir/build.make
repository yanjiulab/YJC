# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

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
CMAKE_SOURCE_DIR = /home/liyj/code/YJC/lib/cmocka-1.1.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liyj/code/YJC/lib/cmocka-1.1.0/build

# Include any dependencies generated for this target.
include example/CMakeFiles/key_value_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include example/CMakeFiles/key_value_test.dir/compiler_depend.make

# Include the progress variables for this target.
include example/CMakeFiles/key_value_test.dir/progress.make

# Include the compile flags for this target's objects.
include example/CMakeFiles/key_value_test.dir/flags.make

example/CMakeFiles/key_value_test.dir/key_value.c.o: example/CMakeFiles/key_value_test.dir/flags.make
example/CMakeFiles/key_value_test.dir/key_value.c.o: ../example/key_value.c
example/CMakeFiles/key_value_test.dir/key_value.c.o: example/CMakeFiles/key_value_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/liyj/code/YJC/lib/cmocka-1.1.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object example/CMakeFiles/key_value_test.dir/key_value.c.o"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT example/CMakeFiles/key_value_test.dir/key_value.c.o -MF CMakeFiles/key_value_test.dir/key_value.c.o.d -o CMakeFiles/key_value_test.dir/key_value.c.o -c /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value.c

example/CMakeFiles/key_value_test.dir/key_value.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/key_value_test.dir/key_value.c.i"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value.c > CMakeFiles/key_value_test.dir/key_value.c.i

example/CMakeFiles/key_value_test.dir/key_value.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/key_value_test.dir/key_value.c.s"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value.c -o CMakeFiles/key_value_test.dir/key_value.c.s

example/CMakeFiles/key_value_test.dir/key_value_test.c.o: example/CMakeFiles/key_value_test.dir/flags.make
example/CMakeFiles/key_value_test.dir/key_value_test.c.o: ../example/key_value_test.c
example/CMakeFiles/key_value_test.dir/key_value_test.c.o: example/CMakeFiles/key_value_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/liyj/code/YJC/lib/cmocka-1.1.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object example/CMakeFiles/key_value_test.dir/key_value_test.c.o"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT example/CMakeFiles/key_value_test.dir/key_value_test.c.o -MF CMakeFiles/key_value_test.dir/key_value_test.c.o.d -o CMakeFiles/key_value_test.dir/key_value_test.c.o -c /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value_test.c

example/CMakeFiles/key_value_test.dir/key_value_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/key_value_test.dir/key_value_test.c.i"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value_test.c > CMakeFiles/key_value_test.dir/key_value_test.c.i

example/CMakeFiles/key_value_test.dir/key_value_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/key_value_test.dir/key_value_test.c.s"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/liyj/code/YJC/lib/cmocka-1.1.0/example/key_value_test.c -o CMakeFiles/key_value_test.dir/key_value_test.c.s

# Object files for target key_value_test
key_value_test_OBJECTS = \
"CMakeFiles/key_value_test.dir/key_value.c.o" \
"CMakeFiles/key_value_test.dir/key_value_test.c.o"

# External object files for target key_value_test
key_value_test_EXTERNAL_OBJECTS =

example/key_value_test: example/CMakeFiles/key_value_test.dir/key_value.c.o
example/key_value_test: example/CMakeFiles/key_value_test.dir/key_value_test.c.o
example/key_value_test: example/CMakeFiles/key_value_test.dir/build.make
example/key_value_test: src/libcmocka.so.0.4.0
example/key_value_test: example/CMakeFiles/key_value_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/liyj/code/YJC/lib/cmocka-1.1.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable key_value_test"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/key_value_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
example/CMakeFiles/key_value_test.dir/build: example/key_value_test
.PHONY : example/CMakeFiles/key_value_test.dir/build

example/CMakeFiles/key_value_test.dir/clean:
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example && $(CMAKE_COMMAND) -P CMakeFiles/key_value_test.dir/cmake_clean.cmake
.PHONY : example/CMakeFiles/key_value_test.dir/clean

example/CMakeFiles/key_value_test.dir/depend:
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liyj/code/YJC/lib/cmocka-1.1.0 /home/liyj/code/YJC/lib/cmocka-1.1.0/example /home/liyj/code/YJC/lib/cmocka-1.1.0/build /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example /home/liyj/code/YJC/lib/cmocka-1.1.0/build/example/CMakeFiles/key_value_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : example/CMakeFiles/key_value_test.dir/depend

