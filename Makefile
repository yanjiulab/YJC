# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
# Modified by Yanjiu Li

###############################################################################
#                                 Main section                                #
###############################################################################

INC_DIRS := ./include
SRC_DIRS := ./src
APP_DIR := ./src/app
BUILD_DIR := ./build
BIN_DIR := ./bin
3RD_DIR := ./3-rd
LIB_DIR := ./lib

# Cross compile (TODO)
# ARCH = arm64
# CROSS_COMPILE ?= aarch64-linux-gnu-
# CROSS = aarch64-linux-gnu-
# CC = $(CROSS_COMPILE)gcc
# LD = $(CROSS_COMPILE)ld

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
APP_FILES=$(shell find $(APP_DIR) -name '*.c')
SRCS := $(filter-out $(APP_FILES), $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s'))

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) $(INC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# Compiler Flags
# -w			: colse all warnings
# -Wall -Wextra	: open all warnings
# -Wxxx			: open specific warning `xxx`
# -Wno-xxx		: close specific warning `xxx`
# -Werror		: treat warnings as errors (not recommanded)
CFLAGS = -g3 -w -Wextra -DPRINT_DEBUG #-std=c99

# Used libraries
LDFLAGS = -L $(LIB_DIR) -lm -ldl -lpthread -lsqlite3 #-lreadline -lncurses -lpanel -lmenu -lform

# All files in the APP_DIR are considered compilation targets and each file should contain the main function.
TARGETS = $(notdir $(patsubst %.c, %, $(APP_FILES)))

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# The final build step will build all the target.
all: $(OBJS)
	@mkdir -pv $(BIN_DIR)
	@for target in $(TARGETS); \
	do					\
	$(CC) $(CPPFLAGS) $(CFLAGS) $(APP_DIR)/$$target.c $(OBJS) -o $(BIN_DIR)/$$target $(LDFLAGS); \
	done

.PHONY: all

clean:
	$(RM) -r $(BUILD_DIR)

cleanLogs:
	$(RM) -r log || true

remove: clean cleanLogs

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

###############################################################################
#                              CMoka test section                             #
###############################################################################

TEST_DIR = ./test
TEST_LIBS = -lcmocka
TESTS = $(shell find $(TEST_DIR) -name '*.c')
TEST_OUTPUT = $(BIN_DIR)/t.out
# TCFLAGS = -Wextra -Wshadow -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-variable -Wno-pragmas -O3 -g3

test-build: $(OBJS)
	echo $(TEST_OUTPUT)
	@mkdir -pv $(BIN_DIR)
	@echo Preparing tests...
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTS) $(OBJS) -o $(TEST_OUTPUT) $(LDFLAGS) $(TEST_LIBS)

test: test-build
	./$(TEST_OUTPUT)
	@echo Tests completed.

###############################################################################
#                           Valgrind check section                            #
###############################################################################

# Leaks log file
LEAKS = log/leaks.log
# Thread chek log file
HELGRIND = log/threads.log
CHECK_BIN = app

leaks:
	@mkdir -p log
	valgrind --leak-check=yes --log-file="$(LEAKS)" --track-origins=yes $(BIN_DIR)/$(CHECK_BIN)

tleaks: test-build
	@mkdir -p log
	valgrind --leak-check=yes --log-file="$(LEAKS)" --track-origins=yes $(TEST_OUTPUT)

threads:
	@mkdir -p log
	valgrind --tool=helgrind --log-file="$(HELGRIND)" $(BIN_DIR)/$(CHECK_BIN)


###############################################################################
#                          3-rd party library section                         #
###############################################################################

SQLITE_DIR = $(3RD_DIR)/sqlite3

sqlite3:
	$(CC) -Os -I$(SQLITE_DIR) -DSQLITE_THREADSAFE=0 -DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE -DSQLITE_ENABLE_EXPLAIN_COMMENTS \
	$(SQLITE_DIR)/shell.c $(SQLITE_DIR)/sqlite3.c -ldl -lm -o $(BIN_DIR)/sqlite3

sqlite3-lib:
	$(CC) -Os -I$(SQLITE_DIR) -DSQLITE_THREADSAFE=2 -DSQLITE_ENABLE_FTS4 \
	-DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_JSON1 \
	-DSQLITE_ENABLE_RTREE -DSQLITE_ENABLE_EXPLAIN_COMMENTS \
	-DHAVE_USLEEP \
	$(SQLITE_DIR)/sqlite3.c -ldl -lm -fPIC -lpthread -shared -o $(LIB_DIR)/libsqlite3.so


###############################################################################
#                          Project generation section                         #
###############################################################################

PROJECT_NAME := project
PROJECT_PATH := ~/projects/$(PROJECT_NAME)
BINARY := project

start:
	@echo "Creating project: $(PROJECT_NAME)"
	@mkdir -pv $(PROJECT_PATH)
	@echo "Copying files from template to new directory:"
	@cp -rvf ./* $(PROJECT_PATH)/
	@echo
	@echo "Go to $(PROJECT_PATH) and compile your project: make"
	@echo "Then execute it: bin/$(BINARY) --help"
	@echo "Happy hacking :-P"

install:
	@echo Installing dependencies...
# Install required libraries here.
	@echo Installed