BENCH_DIR=bench/src
BENCH_SRC=$(foreach src, $(wildcard $(BENCH_DIR)/*.c),$(notdir $(src)))
BENCH_OBJ=$(BENCH_SRC:%.c=$(OBJ_DIR)/%.o)

bench: build $(LIB_DIR)/$(LIB_NAME).a bench_ciphers
	-mkdir -p bench/results/data
	-mkdir -p bench/results/fig

bench_ciphers: $(BENCH_OBJ)
	$(CC) -o $(EXE_DIR)/$@ $^ $(LDFLAGS) $(LDLIBS) -pthread

# compile test sources
$(OBJ_DIR)/%.o: $(BENCH_DIR)/%.c
	$(CC) -I$(HEADERS) -I$(BENCH_DIR) -O3 -c $< -o $@