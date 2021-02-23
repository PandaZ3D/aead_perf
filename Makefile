# make must be run relative to root of directory
BUILD_DIR = build
SRC_DIR = ciphers
TEST_DIR = tests
OBJ_DIR = $(BUILD_DIR)/objs
LIB_DIR = $(BUILD_DIR)/lib
EXE_DIR = $(BUILD_DIR)/bin
HEADERS = include

AR = ar
ARFLAGS = -r -c

CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -Wextra -I$(HEADERS) -g
LDFLAGS = -L$(LIB_DIR)
LDLIBS = -laead

LIB_NAME	= libaead
LIB_SRC 	= $(SRC_DIR)/rjindael/aes.c
LIB_OBJ 	= $(LIB_SRC:$(SRC_DIR)/rjindael/%.c=$(OBJ_DIR)/%.o)


.PHONY: all clean

all: build $(LIB_NAME) test

$(LIB_NAME): build $(LIB_DIR)/$(LIB_NAME).a

build:
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(LIB_DIR)
	-mkdir -p $(EXE_DIR)

$(LIB_DIR)/$(LIB_NAME).a: $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

# gcc -Iinclude tests/test_aes.c -Lbuild/lib/ -laead
test: $(OBJ_DIR)/test_aes.o
	$(CC) -o $(EXE_DIR)/aes_test $< $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -rf $(BUILD_DIR)