TEST_DIR = tests

test: build aes_test chacha20_test aes_ctr_test

aes_test: $(OBJ_DIR)/test_aes.o build $(LIB_DIR)/$(LIB_NAME).a
	$(CC) -o $(EXE_DIR)/$@ $< $(LDFLAGS) $(LDLIBS)

aes_ctr_test: $(OBJ_DIR)/test_aes.o build $(LIB_DIR)/$(LIB_NAME).a
	$(CC) -o $(EXE_DIR)/$@ $< $(LDFLAGS) $(LDLIBS)

chacha20_test: $(OBJ_DIR)/test_chacha20.o build $(LIB_DIR)/$(LIB_NAME).a
	$(CC) -o $(EXE_DIR)/$@ $< $(LDFLAGS) $(LDLIBS)

# compile test sources
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
