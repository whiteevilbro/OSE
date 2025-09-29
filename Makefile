# =============================================================================
# Variables

# Build tools
NASM = nasm -f bin 
SHELL := /bin/bash
VERSION = string

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

BYTES = 29
OFFSET = $$((0x7C00 + $(BYTES)))
SKIP = $$((510 - $(BYTES)))
DATA = 16

d: dis
dis: disasm
disasm: build
	$(NASM) src/helloworld_$(VERSION).asm -o ./boot.bin
	$(eval HLT := 0x$(shell (ndisasm -b 16 -o 0x7c00 -a -p intel ./boot.bin) | (grep -m 1 -P -o '[0-9A-F]{8}(?=\s+F4\s+hlt)' /dev/stdin)))
	ndisasm -b 16 -o 0x7c00 -k $(OFFSET),$(SKIP) -k 0x7C02,$(DATA) -s 0x7C12 -a -p intel ./boot.bin
	echo Estimated bytes: $$(($(HLT) + 1 - 0x7C00 - $(DATA)))

.PHONY: all build clean test debug d dis disasm
