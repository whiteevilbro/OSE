# =============================================================================
# Variables

SHELL = /bin/bash
NULL = /dev/null

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

# Location
BUILD_DIR = build
SOURCE_DIR = src

OBJ_DIR = $(BUILD_DIR)/objects

# Build tools and options
NASM = nasm
NASM_FLAGS = -felf32 -g

GCC = gcc
MAIN_FLAGS = -xc -std=c99 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -masm=intel
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Wno-implicit-fallthrough
DEBUG_FLAGS = -O0 -g3 -D_DEBUG
RELEASE_FLAGS = -O2 -Werror
GCC_FLAGS = $(MAIN_FLAGS) $(WARNINGS_FLAGS)

LD = ld
LINKER_SCRIPT_TEMPLATE = link.ld
LD_FLAGS = -m i386pe --image-base=0

# Sources and headers
ASM_SOURCES = $(shell find -L ./$(SOURCE_DIR) -iname "*.asm")
C_SOURCES = $(shell find -L ./$(SOURCE_DIR) -iname "*.c")
C_HEADERS = $(shell find -L ./$(SOURCE_DIR) -iname "*.h")

ASM_OBJECTS = $(patsubst ./$(SOURCE_DIR)/%.asm, ./$(OBJ_DIR)/%.o, $(ASM_SOURCES))
C_OBJECTS = $(patsubst ./$(SOURCE_DIR)/%.c, ./$(OBJ_DIR)/%.o, $(C_SOURCES))

OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

GCC_FLAGS += $(addprefix -I, $(call uniq,$(dir $(C_HEADERS))))
GCC_FLAGS += $(MODE_FLAGS)

# QEMU
QEMU = qemu-system-i386
QEMU_FLAGS = -cpu pentium2 -m 1g -monitor stdio -device VGA -no-shutdown -no-reboot
QEMU_BOOT_DEVICE = -drive if=floppy,index=0,format=raw,file=boot.img

# maximum kernel size in kB
KERNEL_SIZE_MAX = 30

# Internal variables
LINKER_SCRIPT = $(BUILD_DIR)/link.ld
BUILD_DIRS = $(addprefix ,$(call uniq,$(BUILD_DIR) $(dir $(C_OBJECTS)) $(dir $(ASM_OBJECTS))))
MAKEFLAGS += --no-print-directory

# =============================================================================
# Tasks

all: kill run-debug

.SECONDEXPANSION:

# Incremental tasks

userspace/user.bin:
	@echo -e "\t\e[1mBuilding user programms\e[0m"
	@$(MAKE) -C userspace/

$(BUILD_DIRS):
	@mkdir -p $@

$(LINKER_SCRIPT): $(LINKER_SCRIPT_TEMPLATE)
	@$(GCC) -E -P -DDIR=$(OBJ_DIR) -x c $(LINKER_SCRIPT_TEMPLATE) > $(LINKER_SCRIPT) 2>$(NULL)
	
boot.img: $(BUILD_DIR)/os.bin $(BUILD_DIR)/kernel_size.check userspace/user.bin | $(BUILD_DIR)
	@echo -e "\t\e[1mMaking image\e[0m"
	@dd if=$(BUILD_DIR)/os.bin of=boot.img conv=notrunc 2>$(NULL)
	@dd if=userspace/user.bin of=boot.img seek=194 bs=512 count=128x4 2>$(NULL)

$(C_OBJECTS): $(OBJ_DIR)/%.o : $(SOURCE_DIR)/%.c | $$(dir $(OBJ_DIR)/%.o)
	@echo -e "\t\e[1mCompiling\e[0m" $(notdir $*.c)
	@$(GCC) $(GCC_FLAGS) -c $(SOURCE_DIR)/$*.c -o $@

$(ASM_OBJECTS): $(OBJ_DIR)/%.o : $(SOURCE_DIR)/%.asm | $$(dir $(OBJ_DIR)/%.o)
	@echo -e "\t\e[1mAssembling\e[0m" $(notdir $*.asm)
	@$(NASM) $(NASM_FLAGS) $(SOURCE_DIR)/$*.asm -o $@

$(BUILD_DIR)/os.elf: $(OBJECTS) $(LINKER_SCRIPT) | $(BUILD_DIR)
	@echo -e "\t\e[1mLinking\e[0m" $(notdir $@)
	@$(LD) $(LD_FLAGS) -T $(LINKER_SCRIPT) $(OBJECTS) -o $(BUILD_DIR)/os.elf

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/os.elf | $(BUILD_DIR)
	@echo -e "\t\e[1mBuilding binary from ELF\e[0m"
	@objcopy -I elf32-i386 -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin

$(BUILD_DIR)/kernel_size.check: $(BUILD_DIR)/os.bin ./check.sh | $(BUILD_DIR)
	@echo -e "\t\e[1mChecking kernel size\e[0m"
	@./check.sh $(BUILD_DIR) $(KERNEL_SIZE_MAX)
	@touch $@

compile_commands.json: Makefile
ifeq ($(shell which bear),"")
	$(warning Cannot make compile-commands.json automatically.)
else
	@$(MAKE) clean >$(NULL)
	@bear -- $(MAKE) $(C_OBJECTS) >$(NULL)
endif

# PHONY Tasks

kill:
	@kill $(shell ps | grep -P -o -m 1 "\d+(?=.*qemu)" | head -1) 2>$(NULL) || true

echo:
	@echo $(dir $(BUILD_DIR))

compile: clean-compile $(C_OBJECTS)
assemble: clean-assemble $(ASM_OBJECTS)
link: clean-link $(BUILD_DIR)/os.elf
bin: clean-bin $(BUILD_DIR)/os.bin
check: $(BUILD_DIR)/kernel_size.check
image: boot.img

clangd: compile_commands.json

clean: clean-compile clean-assemble clean-link clean-bin clean-image clean-check
	@rm -rf $(BUILD_DIR) 
	@$(MAKE) -C userspace/ clean

clean-compile:
	@rm -f $(C_OBJECTS)

clean-assemble:
	@rm -f $(ASM_OBJECTS)

clean-link:
	@rm -f $(BUILD_DIR)/*.elf

clean-bin:
	@rm -f $(BUILD_DIR)/*.bin

clean-image:
	@rm -f *.img

clean-check:
	@rm -f $(BUILD_DIR)/*.check

# ===== BUILD MODES =====

$(BUILD_DIR)/.RELEASE: | $(BUILD_DIR)
	touch $@

$(BUILD_DIR)/.DEBUG: | $(BUILD_DIR)
	touch $@

build-debug: GCC_FLAGS += $(DEBUG_FLAGS)
build-debug: $(BUILD_DIR)/.DEBUG
build-debug: boot.img

build-release: GCC_FLAGS += $(RELEASE_FLAGS)
build-release: $(BUILD_DIR)/.RELEASE
build-release: boot.img

test: run-debug
run-debug: build-debug
	@echo -e "\t\e[1mRunning debug\e[0m"
	@$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE)

debug: boot.img
	@echo -e "\t\e[1mDebugging\e[0m"
	@$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE) -s -S &
	gdb
	@$(MAKE) kill

.PHONY: all test release clean debug compile assemble link check kill clean-compile clean-assemble clean-link clean-bin clean-image clean-check clangd
.PHONY: build-release run-release build-debug run-debug build-
