# =============================================================================
# Variables

# Build tools
NASM = nasm -f bin -g

# kB to load
N = 481


# =============================================================================
# Tasks

all: clean build test

.tmp/boot.bin: src/boot.asm
	$(NASM) src/boot.asm -o .tmp/boot.bin -DN=$(N)

boot.img: .tmp/boot.bin
	dd if=/dev/random of=boot.img bs=1024 count=$(N)
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
