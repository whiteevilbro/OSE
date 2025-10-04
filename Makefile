# =============================================================================
# Variables

SHELL = /bin/bash

# Build tools and options
NASM = nasm -felf32 -g

GCC = gcc
MAIN_FLAGS = -std=c99 -O0 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -g3
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Wno-implicit-fallthrough
GCC_FLAGS = $(MAIN_FLAGS) $(WARNINGS_FLAGS)

LD = ld
LINKER_SCRIPT = link.ld
LD_FLAGS = -m i386pe --image-base=0

# Sources and headers
ASM_SOURCES = $(wildcard ./*.asm) $(wildcard **/*.asm)
C_SOURCES = $(wildcard ./*.c) $(wildcard **/*.c)
HEADERS = $(wildcard ./*.h) $(wildcard **/*.h)
GCC_FLAGS += $(addprefix -I, $(dir $(HEADERS)))

BUILD_DIR = build

# QEMU
QEMU = qemu-system-i386
QEMU_FLAGS = -cpu pentium2 -m 1g -monitor stdio -device VGA --no-reboot

# maximum kernel size in kB
KERNEL_SIZE_MAX = 20

# =============================================================================
# Tasks

all: kill clean build test

kill:
	kill $(shell ps | grep -P -o -m 1 "\d+(?=.*qemu)" | head -1) 2>/dev/null || true

build: boot.img
boot.img: $(BUILD_DIR)/os.bin check
	@dd if=/dev/zero of=$(BUILD_DIR)/os.img bs=1024 count=$$(((ACTUAL_KERNEL_SIZE / 1024) + (ACTUAL_KERNEL_SIZE % 1024) != 0))
	@dd if=$(BUILD_DIR)/os.bin of=boot.img conv=notrunc

compile: $(BUILD_DIR)/kernel.o
$(BUILD_DIR)/kernel.o: $(C_SOURCES)
	@echo -e "\t\e[1mCompiling\e[0m"
	$(GCC) $(GCC_FLAGS) -c $(C_SOURCES) -o $(BUILD_DIR)/kernel.o

assemble: $(BUILD_DIR)/boot.o
$(BUILD_DIR)/boot.o: $(ASM_SOURCES)
	@echo -e "\t\e[1mAssembling\e[0m"
	$(NASM) $(ASM_SOURCES) -o $(BUILD_DIR)/boot.o

link: $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot.o $(BUILD_DIR)/os.elf
$(BUILD_DIR)/os.elf: $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(LINKER_SCRIPT)
	@echo -e "\t\e[1mLinking\e[0m"
	@$(GCC) $(BUILD_DIR)/boot.o -E -P -DBUILD_DIR=$(BUILD_DIR) -x c $(LINKER_SCRIPT) > $(BUILD_DIR)/$(LINKER_SCRIPT) 2>/dev/null
	$(LD) $(LD_FLAGS) -T $(BUILD_DIR)/$(LINKER_SCRIPT) $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o -o $(BUILD_DIR)/os.elf

bin: $(BUILD_DIR)/os.bin
$(BUILD_DIR)/os.bin: compile assemble link
	@echo -e "\t\e[1mBuilding binary from ELF\e[0m"
	objcopy -I elf32-i386 -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin

check: $(BUILD_DIR)/os.bin
	@echo -e "\t\e[1mChecking kernel size\e[0m"
	$(eval ACTUAL_KERNEL_SIZE := $(shell wc -c < ./$(BUILD_DIR)/os.bin))
	@echo -e "\t\e[1mKERNEL SIZE: $(ACTUAL_KERNEL_SIZE)\e[0m"
	@if [ $(ACTUAL_KERNEL_SIZE) -le $$((KERNEL_SIZE_MAX * 1024)) ]; then\
		@echo EXPECTED_KERNEL_SIZE: $(KERNEL_SIZE) kb;\
		@echo ACTUAL_KERNEL_SIZE: $$((ACTUAL_KERNEL_SIZE / 1024)) kB;\
		exit 127;\
	fi

clean:
	rm -f *.img
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)

test: boot.img
	$(QEMU) $(QEMU_FLAGS) -drive if=floppy,index=0,format=raw,file=boot.img

debug: kill clean boot.img
	$(QEMU) $(QEMU_FLAGS) -drive if=floppy,index=0,format=raw,file=boot.img -s -S &
	gdb

.PHONY: all build clean test debug compile assemble link check kill
