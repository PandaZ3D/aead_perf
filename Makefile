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
LIB_SRC 	= $(foreach src, $(wildcard $(SRC_DIR)/*/*.c),$(notdir $(src)))
LIB_OBJ 	= $(LIB_SRC:%.c=$(OBJ_DIR)/%.o)

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
test: build aes_test chacha20_test

aes_test: $(OBJ_DIR)/test_aes.o build $(LIB_DIR)/$(LIB_NAME).a
	$(CC) -o $(EXE_DIR)/aes_test $< $(LDFLAGS) $(LDLIBS)

chacha20_test: $(OBJ_DIR)/test_chacha20.o build $(LIB_DIR)/$(LIB_NAME).a
	$(CC) -o $(EXE_DIR)/chacha20_test $< $(LDFLAGS) $(LDLIBS)

# compile lib sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# compile test sources
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -rf $(BUILD_DIR)
