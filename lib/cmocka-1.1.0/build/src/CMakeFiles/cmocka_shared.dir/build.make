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
include src/CMakeFiles/cmocka_shared.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/cmocka_shared.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/cmocka_shared.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/cmocka_shared.dir/flags.make

src/CMakeFiles/cmocka_shared.dir/cmocka.c.o: src/CMakeFiles/cmocka_shared.dir/flags.make
src/CMakeFiles/cmocka_shared.dir/cmocka.c.o: ../src/cmocka.c
src/CMakeFiles/cmocka_shared.dir/cmocka.c.o: src/CMakeFiles/cmocka_shared.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/liyj/code/YJC/lib/cmocka-1.1.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/CMakeFiles/cmocka_shared.dir/cmocka.c.o"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cmocka_shared.dir/cmocka.c.o -MF CMakeFiles/cmocka_shared.dir/cmocka.c.o.d -o CMakeFiles/cmocka_shared.dir/cmocka.c.o -c /home/liyj/code/YJC/lib/cmocka-1.1.0/src/cmocka.c

src/CMakeFiles/cmocka_shared.dir/cmocka.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cmocka_shared.dir/cmocka.c.i"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/liyj/code/YJC/lib/cmocka-1.1.0/src/cmocka.c > CMakeFiles/cmocka_shared.dir/cmocka.c.i

src/CMakeFiles/cmocka_shared.dir/cmocka.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cmocka_shared.dir/cmocka.c.s"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/liyj/code/YJC/lib/cmocka-1.1.0/src/cmocka.c -o CMakeFiles/cmocka_shared.dir/cmocka.c.s

# Object files for target cmocka_shared
cmocka_shared_OBJECTS = \
"CMakeFiles/cmocka_shared.dir/cmocka.c.o"

# External object files for target cmocka_shared
cmocka_shared_EXTERNAL_OBJECTS =

src/libcmocka.so.0.4.0: src/CMakeFiles/cmocka_shared.dir/cmocka.c.o
src/libcmocka.so.0.4.0: src/CMakeFiles/cmocka_shared.dir/build.make
src/libcmocka.so.0.4.0: src/CMakeFiles/cmocka_shared.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/liyj/code/YJC/lib/cmocka-1.1.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library libcmocka.so"
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cmocka_shared.dir/link.txt --verbose=$(VERBOSE)
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && $(CMAKE_COMMAND) -E cmake_symlink_library libcmocka.so.0.4.0 libcmocka.so.0 libcmocka.so

src/libcmocka.so.0: src/libcmocka.so.0.4.0
	@$(CMAKE_COMMAND) -E touch_nocreate src/libcmocka.so.0

src/libcmocka.so: src/libcmocka.so.0.4.0
	@$(CMAKE_COMMAND) -E touch_nocreate src/libcmocka.so

# Rule to build all files generated by this target.
src/CMakeFiles/cmocka_shared.dir/build: src/libcmocka.so
.PHONY : src/CMakeFiles/cmocka_shared.dir/build

src/CMakeFiles/cmocka_shared.dir/clean:
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src && $(CMAKE_COMMAND) -P CMakeFiles/cmocka_shared.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/cmocka_shared.dir/clean

src/CMakeFiles/cmocka_shared.dir/depend:
	cd /home/liyj/code/YJC/lib/cmocka-1.1.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liyj/code/YJC/lib/cmocka-1.1.0 /home/liyj/code/YJC/lib/cmocka-1.1.0/src /home/liyj/code/YJC/lib/cmocka-1.1.0/build /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src /home/liyj/code/YJC/lib/cmocka-1.1.0/build/src/CMakeFiles/cmocka_shared.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/cmocka_shared.dir/depend

