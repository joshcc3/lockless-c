COMPILER_FLAGS := -Wall -std=c11 -g -mcx16 -O2
COMPILER_LIBS :=  -Llibs -ljc -lpthread -lm -latomic
COMPILER_INCLUDES := -Iinclude
COMPILER := gcc
SRC_DIR :=  src
APP_NAME := concurrent/atomic_snapshot/wait_free/unbounded_register
LIB_ROOT := $(SRC_DIR)/$(APP_NAME)
OUT_ROOT := out

COMPILER_CMD := $(COMPILER) $(COMPILER_FLAGS)

source_files := $(shell cd src && find . -type d -name 'experiment' -prune -o -type f -name "*.c" -print)

all: unbounded_regs


$(APP_NAME)/%.o: src/$(APP_NAME)/%.c
	mkdir -p $(OUT_ROOT)/$(APP_NAME)
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@ $(COMPILER_LIBS)

concurrent/%.o: src/concurrent/%.c
	echo $(source_files)
	mkdir -p $(OUT_ROOT)/concurrent
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@  $(COMPILER_LIBS)

unbounded_regs: $(source_files:%.c=%.o)
	echo $^
	mkdir -p $(OUT_ROOT)
	$(COMPILER_CMD)  $(COMPILER_INCLUDES) $(shell find out -type f -name "*.o")  -I$(SRC_DIR) -o out/unbounded_regs $(COMPILER_LIBS)

.PHONY clean:
clean:
	rm -rf $(OUT_ROOT)
