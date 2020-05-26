CC = gcc
CFLAGS = -Wall -O2 -lm
INCLUDE_DIR = -Iinclude

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)

# FOR COMPILE
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEMO_SRCS = $(shell find demo/ -name "*.c")
DEMO_OBJS = $(DEMO_SRCS:%.c=$(BUILD_DIR)/%.o)

# *.c -> BUILD/*.o
$(BUILD_DIR)/%.o: %.c 
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_DIR)

# FOR LINKAGE
demo/test.o: $(BUILD_DIR)/demo/test.o $(OBJS)
	@echo link $^
	$(CC) $^ -o $@ $(CFLAGS)

.PHONY: clean test
clean:
	find . -name "*.o" | xargs rm -f

test: clean demo/test.o
	demo/test.o
