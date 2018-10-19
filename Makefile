COMPILER_FLAGS := -Wall -std=c11 -lpthread -lm -latomic
COMPILER := gcc
PROJECT_ROOT := ${proj_root}
SRC_DIR :=  $(PROJECT_ROOT)/src
APP_NAME := concurrent/wait_free/atomic_snapshot/unbounded_register/
LIB_ROOT := $(SRC_DIR)/$(APP_NAME)
OUT_ROOT := $(PROJECT_ROOT)/out

COMPILER_CMD := $(COMPILER) $(COMPILER_FLAGS) 


all: unbounded_regs

$(OUT_ROOT)/concurrent/%.o: src/concurrent/%.c
	mkdir -p $(OUT_ROOT)/concurrent
	$(COMPILER_CMD) -I . $^ -c -o $@

$(OUT_ROOT)/$(APP_NAME)/%.o: src/$(APP_NAME)/%.c
	mkdir -p $(OUT_ROOT)/$(APP_NAME)
	$(COMPILER_CMD) -I . $^ -c -o $@

unbounded_regs: $(OUT_ROOT)/$(APP_NAME)/main.o $(OUT_ROOT)/$(APP_NAME)/snapshot_object.o $(OUT_ROOT)/$(APP_NAME)/conc.o $(OUT_ROOT)/concurrent/atomic.o
	mkdir -p $(OUT_ROOT)
	$(COMPILER_CMD) $^ -o $(OUT_ROOT)/$@

.PHONY clean:
clean:
	rm -rf $(PROJECT_ROOT)/out

