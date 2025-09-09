# =============================================================================
# Variables

# Build tools
NASM = nasm -f bin 


# =============================================================================
# Tasks

all: clean build test

.tmp/boot.bin: src/boot.asm
	$(NASM) src/boot.asm -o .tmp/boot.bin

boot.img: .tmp/boot.bin
	dd if=/dev/zero of=boot.img bs=1024 count=1440
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

.PHONY: all build clean test debug
