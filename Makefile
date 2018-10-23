COMPILER_FLAGS := -Wall -std=c11 -g -mcx16 -O2
COMPILER_LIBS :=  -Llibs -ljc -lpthread -lm -latomic
COMPILER_INCLUDES := -Iinclude
COMPILER := gcc
SRC_DIR :=  src
APP_NAME := concurrent/atomic_snapshot/wait_free/unbounded_register
LIB_ROOT := $(SRC_DIR)/$(APP_NAME)
OUT_ROOT := out

COMPILER_CMD := $(COMPILER) $(COMPILER_FLAGS)

source_files := $(shell cd src && find concurrent -type d -name 'experiment' -prune -o -type f -name "*.c" -print)

all: ao.out


$(APP_NAME)/%.o: src/$(APP_NAME)/%.c
	echo "case 1"
	mkdir -p $(OUT_ROOT)/$(APP_NAME)
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@ $(COMPILER_LIBS)

concurrent/atomic_snapshot/%.o: src/concurrent/atomic_snapshot/%.c
	echo "case 2"
	mkdir -p $(OUT_ROOT)/concurrent/atomic_snapshot
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@  $(COMPILER_LIBS)
concurrent/atomic_snapshot/locking/%.o: src/concurrent/atomic_snapshot/locking/%.c
	echo "case 2"
	mkdir -p $(OUT_ROOT)/concurrent/atomic_snapshot/locking
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@  $(COMPILER_LIBS)

concurrent/%.o: src/concurrent/%.c
	echo "case 3"
	mkdir -p $(OUT_ROOT)/concurrent
	$(COMPILER_CMD) $(COMPILER_INCLUDES) -I$(SRC_DIR) $^ -c -o out/$@  $(COMPILER_LIBS)

ao.out: $(source_files:%.c=%.o) src/main.c
	mkdir -p $(OUT_ROOT)
	$(COMPILER_CMD)  $(COMPILER_INCLUDES) $(shell find out/concurrent -type f -name "*.o") src/main.c -I$(SRC_DIR) -o out/ao.out $(COMPILER_LIBS)

#locking: $(source_files:%.c=%.o)
#	mkdir -p $(OUT_ROOT)
#	$(COMPILER_CMD) $(COMP


.PHONY clean:
clean:
	rm -rf $(OUT_ROOT)
