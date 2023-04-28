# C Makefile using gcc, gdb and valgrind. 
# Modified version of Makefile using g++ & gdb by Roberto Nicolas Savinelli <rsavinelli@est.frba.utn.edu.ar>
# Tomas Agustin Sanchez <tosanchez@est.frba.utn.edu.ar>

# Includes the project configurations
include project.conf

# Validating project variables defined in project.conf
ifndef PROJECT_NAME
$(error Missing PROJECT_NAME. Put variables at project.conf file)
endif
ifndef BINARY
$(error Missing BINARY. Put variables at project.conf file)
endif
ifndef PROJECT_PATH
$(error Missing PROJECT_PATH. Put variables at project.conf file)
endif

# C Compiler
CC = gcc
# Compiler Flags
# CFLAGS =  -std=c99 #-Wall -g3
# Test Compiler flags
TCFLAGS = -g3 -DPRINT_DEBUG
# TCFLAGS = -Wextra -Wshadow -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-pragmas -O3 -g3
# TCFLAGS = -Wall -Wextra -Wshadow -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-pragmas -O3 -g3
# Used libraries
LIBS = -lm -lreadline -lpthread -lncurses -lsqlite3 -ldl #-L lib
# Test libraries
TEST_LIBS = -lcmocka -L /usr/lib 
# Include directory
INCLUDE_DIRECTORY=./include/ ./src/
# Source directory
SOURCE_DIRECTORY=./src
# Test Directory
TEST_DIRECTORY=./test
# The main file path
MAIN_FILE= ./src/app/main.c
# Inlcude folder
INCLUDES = $(foreach dir, $(shell find $(INCLUDE_DIRECTORY) -type d -print), $(addprefix -I , $(dir)))
# Source files
SOURCES = $(filter-out $(MAIN_FILE), $(shell find $(SOURCE_DIRECTORY) -name '*.c'))
# Test cases files
TESTS = $(shell find $(TEST_DIRECTORY) -name '*.c')
# Application name
APPNAME = route
# Output file name
OUTPUT = build/$(APPNAME).out
# Test Output file
TEST_OUTPUT = build/$(APPNAME)_test.out
# Leaks log file
LEAKS = log/leaks.log
# Thread chek log file
HELGRIND = log/threads.log


all : compile run

.PHONY: all

start:
	@echo "Creating project: $(PROJECT_NAME)"
	@mkdir -pv $(PROJECT_PATH)
	@echo "Copying files from template to new directory:"
	@cp -rvf ./* $(PROJECT_PATH)/
	@echo
	@echo "Go to $(PROJECT_PATH) and compile your project: make"
	@echo "Then execute it: build/$(BINARY) --help"
	@echo "Happy hacking :-P"

install:
	@echo Installing dependencies...
# Install required libraries here.
	@echo Installed

dirs: 
	@echo $(DIRS)

compile:
	@mkdir -p build
	@echo Building...
	$(CC) $(CFLAGS) $(MAIN_FILE) $(SOURCES) $(INCLUDES) $(LIBS) -o $(OUTPUT)
	@echo Build completed.

run: compile
	@echo 
	./$(OUTPUT)

test-build:
	@mkdir -p build
	@echo Preparing tests...
	$(CC) $(TCFLAGS) $(TESTS) $(SOURCES) $(INCLUDES) $(LIBS) $(TEST_LIBS) -o $(TEST_OUTPUT)

test: test-build
	./$(TEST_OUTPUT)
	@echo Tests completed.
	
leaks: compile
	@mkdir -p log
	valgrind --leak-check=yes --log-file="$(LEAKS)" --track-origins=yes ./$(OUTPUT)

threads: compile
	@mkdir -p log
	valgrind --tool=helgrind --log-file="$(HELGRIND)" ./$(OUTPUT)

clean:
	$(RM) ./$(OUTPUT)

cleanLogs:
	$(RM) -r log || true

remove: clean cleanLogs