# make must be run relative to root of directory
BUILD_DIR = build
SRC_DIR = ciphers
OBJ_DIR = $(BUILD_DIR)/objs
LIB_DIR = $(BUILD_DIR)/lib
HEADERS = include

CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -Wextra -I$(HEADERS) -g
LDFLAGS = -L$(LIB_DIR) -laead
# LDLIBS = 

LIB_NAME	= libaead
LIB_SRC 	= $(SRC_DIR)/rjindael/aes.c
LIB_OBJ 	= $(LIB_SRC:$(SRC_DIR)/rjindael/%.c=$(OBJ_DIR)/%.o)


.PHONY: all clean

all: build $(LIB_NAME) test

$(LIB_NAME): build $(LIB_DIR)/$(LIB_NAME).a

build:
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(LIB_DIR)

$(LIB_DIR)/$(LIB_NAME).a: $(LIB_OBJ)
	ar -rc $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: tests/test_aes.o
	$(CC) -o test_aes tests/test_aes.o $(LDFLAGS)

tests/test_aes.o: tests/test_aes.c
	$(CC) $(CFLAGS) tests/test_aes.c

clean:
	rm -rf $(BUILD_DIR)