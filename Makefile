# =============================================================================
# Variables

SHELL = /bin/bash

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

# Location
BUILD_DIR = build
SOURCE_DIR = src

# Build tools and options
NASM = nasm
NASM_FLAGS = -felf32 -g

GCC = gcc
MAIN_FLAGS = -std=c99 -O0 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -g3 -DDEBUG -masm=intel
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Wno-implicit-fallthrough -Werror
GCC_FLAGS = $(MAIN_FLAGS) $(WARNINGS_FLAGS)

MAIN_FLAGS += $(shell [ $(shell $(GCC) -dumpversion) -lt 15 ] && echo "-mno-red-zone" || echo "") 


LD = ld
LINKER_SCRIPT = link.ld
LD_FLAGS = -m i386pe --image-base=0

# Sources and headers
ASM_SOURCES = $(shell find -L ./$(SOURCE_DIR) -iname "*.asm")
C_SOURCES = $(shell find -L ./$(SOURCE_DIR) -iname "*.c")
C_HEADERS = $(shell find -L ./$(SOURCE_DIR) -iname "*.h")

ASM_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(patsubst ./$(SOURCE_DIR)/%.asm, %.o, $(ASM_SOURCES))))
C_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(patsubst ./$(SOURCE_DIR)/%.c, %.o, $(C_SOURCES))))

GCC_FLAGS += $(addprefix -I, $(call uniq,$(dir $(C_HEADERS))))

# QEMU
QEMU = qemu-system-i386
QEMU_FLAGS = -cpu pentium2 -m 1g -monitor stdio -device VGA -no-shutdown -no-reboot
QEMU_BOOT_DEVICE = -drive if=floppy,index=0,format=raw,file=boot.img

# maximum kernel size in kB
KERNEL_SIZE_MAX = 20

# =============================================================================
# Tasks

all: kill clean build test

# Incremental tasks

boot.img: $(BUILD_DIR)/os.bin $(BUILD_DIR)/kernel_size.check
	@echo -e "\t\e[1mMaking image\e[0m"
	@dd if=$(BUILD_DIR)/os.bin of=boot.img conv=notrunc

$(C_OBJECTS): $(C_SOURCES) $(C_HEADERS)
	@echo -e "\t\e[1mCompiling\e[0m" $(notdir $*)
	$(GCC) $(GCC_FLAGS) -c $(shell find -L ./$(SOURCE_DIR)/ -iname "$(notdir $*).c") -o $@

$(ASM_OBJECTS): $(ASM_SOURCES)
	@echo -e "\t\e[1mAssembling\e[0m" $(notdir $*)
	$(NASM) $(NASM_FLAGS) $(shell find -L ./$(SOURCE_DIR)/ -iname "$(notdir $*).asm") -o $@

$(BUILD_DIR)/os.elf: $(ASM_OBJECTS) $(C_OBJECTS) $(LINKER_SCRIPT)
	@echo -e "\t\e[1mLinking\e[0m"
	@$(GCC) $(BUILD_DIR)/boot.o -E -P -DBUILD_DIR=$(BUILD_DIR) -x c $(LINKER_SCRIPT) > $(BUILD_DIR)/$(LINKER_SCRIPT) 2>/dev/null
	$(LD) $(LD_FLAGS) -T $(BUILD_DIR)/$(LINKER_SCRIPT) $(BUILD_DIR)/boot.o $(C_OBJECTS) -o $(BUILD_DIR)/os.elf

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/os.elf
	@echo -e "\t\e[1mBuilding binary from ELF\e[0m"
	objcopy -I elf32-i386 -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin

$(BUILD_DIR)/kernel_size.check: $(BUILD_DIR)/os.bin ./check.sh
	@echo -e "\t\e[1mChecking kernel size\e[0m"
	@./check.sh $(BUILD_DIR) $(KERNEL_SIZE_MAX)
	touch $@

# PHONY Tasks

kill:
	kill $(shell ps | grep -P -o -m 1 "\d+(?=.*qemu)" | head -1) 2>/dev/null || true

build: boot.img
echo:
	@echo ECHO: $(MAIN_FLAGS)

compile: clean-compile $(C_OBJECTS)
assemble: clean-assemble $(ASM_OBJECTS)
link: clean-link $(BUILD_DIR)/os.elf
bin: clean-bin $(BUILD_DIR)/os.bin
check: $(BUILD_DIR)/kernel_size.check

clean: clean-compile clean-assemble clean-link clean-bin clean-image clean-check
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)

clean-compile:
	rm -f $(C_OBJECTS)

clean-assemble:
	rm -f $(ASM_OBJECTS)

clean-link:
	rm -f $(BUILD_DIR)/*.elf

clean-bin:
	rm -f $(BUILD_DIR)/*.bin

clean-image:
	rm -f *.img

clean-check:
	rm -f $(BUILD_DIR)/*.check

test: boot.img
	@echo -e "\t\e[1mRunning\e[0m"
	$(QEMU) $(QEMU_FLAGS) -drive if=floppy,index=0,format=raw,file=boot.img

debug: kill clean boot.img
	@echo -e "\t\e[1mRunning debug\e[0m"
	$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE) -s -S &
	gdb

.PHONY: all build clean test debug compile assemble link check kill clean-compile clean-assemble clean-link clean-bin clean-image clean-check
