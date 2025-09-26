# =============================================================================
# Variables

# Build tools
NASM = nasm -f bin 


# =============================================================================
# Tasks

all: clean build test

.tmp/boot.bin: src/helloworld.asm
	$(NASM) src/helloworld.asm -o .tmp/boot.bin -DN=$(N)

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

dis: build
	$(NASM) src/helloworld.asm -o ./boot.bin
	ndisasm -b 16 -o 0x7c00 -k0,64 -p intel ./boot.bin

.PHONY: all build clean test debug
