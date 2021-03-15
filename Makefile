# make must be run relative to root of directory
BUILD_DIR = build
SRC_DIR = ciphers
OBJ_DIR = $(BUILD_DIR)/objs
LIB_DIR = $(BUILD_DIR)/lib
EXE_DIR = $(BUILD_DIR)/bin
HEADERS = include

AR = ar
ARFLAGS = -r -c

DEBUG_FLAGS = -Wall -Wpedantic -Werror -Wextra -g
SIMD_FLAGS = -msse -msse2 -msse3 -msse4.1 -mavx -mavx2

CC = gcc
CFLAGS = $(SIMD_FLAGS) $(DEBUG_FLAGS) -I$(HEADERS) #-O3 makes it faster...
LDFLAGS = -L$(LIB_DIR)
LDLIBS = -laead

LIB_NAME	= libaead
LIB_SRC 	= $(foreach src, $(wildcard $(SRC_DIR)/*/*.c),$(notdir $(src)))
LIB_OBJ 	= $(LIB_SRC:%.c=$(OBJ_DIR)/%.o)

-include tests/Makefile
-include bench/Makefile

.PHONY: all clean

all: build $(LIB_NAME) test bench

$(LIB_NAME): build $(LIB_DIR)/$(LIB_NAME).a

build:
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(LIB_DIR)
	-mkdir -p $(EXE_DIR)

$(LIB_DIR)/$(LIB_NAME).a: $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

# compile lib sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -rf $(BUILD_DIR)
