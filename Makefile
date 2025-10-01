# =============================================================================
# Variables

# Build tools and options
NASM = nasm -felf32

GCC = gcc
MAIN_FLAGS = -std=c99 -O0 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Wno-implicit-fallthrough
GCC_FLAGS = $(MAIN_FLAGS) $(WARNINGS_FLAGS)

LD = ld
LINKER_SCRIPT = link.ld
LD_FLAGS = -m i386pe -T $(LINKER_SCRIPT)

# Sources and headers
ASM_SOURCES = $(wildcard ./*.asm) $(wildcard **/*.asm)
C_SOURCES = $(wildcard ./*.c) $(wildcard **/*.c)
HEADERS = $(wildcard ./*.h) $(wildcard **/*.h)
GCC_FLAGS += $(addprefix -I, $(dir $(HEADERS)))


# QEMU

QEMU = qemu-system-i386
QEMU_FLAGS = -cpu pentium2 -m 1g -fda boot.img -monitor stdio -device VGA

# kernel size in kB
KERNEL_SIZE = 20


# =============================================================================
# Tasks

all: clean build test

boot.img: .tmp/os.bin
	dd if=/dev/zero of=os.img bs=1024 count=$(KERNEL_SIZE)
	dd if=.tmp/os.bin of=boot.img conv=notrunc

build: boot.img
ACTUAL_KERNEL_SIZE = 0
.tmp/os.bin:
	$(GCC) $(GCC_FLAGS) -c $(C_SOURCES) -o .tmp/kernel.o
	$(NASM) $(ASM_SOURCES) -o .tmp/boot.o -DKERNEL_SIZE=$(KERNEL_SIZE)
	$(LD) $(LD_FLAGS) .tmp/boot.o .tmp/kernel.o -o .tmp/os.elf
	
	objcopy -I elf32-i386 -O binary .tmp/os.elf .tmp/os.bin
# 	$(eval ACTUAL_KERNEL_SIZE := $(shell wc -c < .tmp/os.bin))
# 	@if [ $(ACTUAL_KERNEL_SIZE) -gt $(KERNEL_SIZE) ] then\
# 		@echo EXPECTED_KERNEL_SIZE: $(KERNEL_SIZE);\
# 		@echo ACTUAL_KERNEL_SIZE: $(ACTUAL_KERNEL_SIZE) kB;\
# 		exit\
# 	fi

clean:
	rm -f *.img
	rm -rf .tmp/*

test: build
	$(QEMU) $(QEMU_FLAGS)

debug: build
	$(QEMU) $(QEMU_FLAGS) -s -S &
	gdb

.PHONY: all build clean test debug
