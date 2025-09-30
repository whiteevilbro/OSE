# =============================================================================
# Variables

# Build tools
NASM = nasm -f bin 
SHELL := /bin/bash
VERSION = video

# =============================================================================
# Tasks

all: clean build test

.tmp/boot.bin: src/helloworld_$(VERSION).asm
	$(NASM) src/helloworld_$(VERSION).asm -o .tmp/boot.bin -DN=$(N)

boot.img: .tmp/boot.bin
	dd if=./payload of=boot.img bs=1024 count=1
	dd if=.tmp/boot.bin of=boot.img conv=notrunc

build: boot.img

clean:
	rm -f *.img
	rm -rf .tmp
	mkdir .tmp

test: build
	qemu-system-i386 -cpu pentium2 -m 1g -fda boot.img -monitor stdio -device VGA

debug: build
	qemu-system-i386 -cpu pentium2 -m 1g -fda boot.img -monitor stdio -device VGA -s -S &
	gdb

# BYTES = 32
# OFFSET = $$((0x7C00 + $(BYTES)))
# SKIP = $$((510 - $(BYTES)))
DATA_START = 0x7C03
DATA = 20

d: dis
dis: disasm
disasm: build
	$(NASM) src/helloworld_$(VERSION).asm -o ./boot.bin
	$(eval HLT := 0x$(shell (ndisasm -b 16 -o 0x7c00 -a -p intel ./boot.bin) | (grep -m 1 -P -o '[0-9A-F]{8}(?=\s+F4\s+hlt)' /dev/stdin)))
	$(eval BYTES := $$$$(( $(HLT) - 0x7C00 + 1)))
	$(eval OFFSET := $$$$((0x7C00 + $$(BYTES))))
	$(eval SKIP := $$$$((510 - $$(BYTES))))
	ndisasm -b 16 -o 0x7c00 -k $(OFFSET),$(SKIP) -k $(DATA_START),$(DATA) -s 0x7C00 -a -p intel ./boot.bin
	@echo Estimated bytes: $$(($(BYTES) - $(DATA)))

.PHONY: all build clean test debug d dis disasm
