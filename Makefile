CC = gcc
CFLAGS = -Wall -O2 -lm
INCLUDE_DIR = -Iinclude

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: src/%.c
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_DIR)

build/test.o: test.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_DIR)

.PHONY: clean test
clean:
	find . -name "*.o" | xargs rm -f

test.o: $(OBJS) build/test.o
	$(CC) $^ -o $@ $(CFLAGS)
