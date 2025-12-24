SHELL = /bin/bash
NULL = /dev/null

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

# Build tools and options
NASM = nasm
NASM_FLAGS = -felf32 -g

GCC = gcc
MAIN_FLAGS = -xc -std=c99 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -masm=intel -O0 -g3
GCC_FLAGS = $(MAIN_FLAGS)

LD = ld
LINKER_SCRIPT = ./link.ld
LD_FLAGS = -m i386pe --image-base=0

# Sources and headers

STD_ASM_SOURCES = $(shell find -L ./std/* -iname "*.asm")
STD_C_SOURCES = $(shell find -L ./std/* -iname "*.c")
STD_HEADERS = $(shell find -L ./std/* -iname "*.h")

STD_ASM_OBJECTS = $(patsubst %.asm, %.o, $(STD_ASM_SOURCES))
STD_C_OBJECTS = $(patsubst %.c, %.o, $(STD_C_SOURCES))

USER_C_SOURCES = $(shell find -L ./ -iname "*.c" -not -path "./std/*")
USER_ASM_SOURCES = $(shell find -L ./ -iname "*.asm" -not -path "./std/*")
USER_C_OBJECTS = $(patsubst %.c, %.o, $(USER_C_SOURCES))
USER_ASM_OBJECTS = $(patsubst %.asm, %.o, $(USER_ASM_SOURCES))

USER_OBJECTS = $(USER_C_OBJECTS) $(USER_ASM_OBJECTS)

ELFS = $(patsubst %.o, %.elf, $(USER_OBJECTS))
STRIPS = $(patsubst %.elf, %.strp, $(ELFS))
BINS = $(patsubst %.strp, %.bin, $(STRIPS))
BIN = user.bin

GCC_FLAGS += $(addprefix -I, $(call uniq,$(dir $(C_HEADERS))))

all: $(BIN)

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.elf" -type f -delete
	find . -name "*.strp" -type f -delete
	find . -name "*.bin" -type f -delete

echo:
	@echo $(STD_ASM_OBJECTS)
	@echo $(STD_C_OBJECTS)

$(USER_C_OBJECTS) : %.o : %.c
	$(GCC) $(GCC_FLAGS) -c $*.c -o $@

$(USER_ASM_OBJECTS): %.o : %.asm
	$(NASM) $(NASM_FLAGS) $*.asm -o $@

$(STD_C_OBJECTS) : %.o : %.c
	$(GCC) $(GCC_FLAGS) -c $*.c -o $@

$(STD_ASM_OBJECTS): %.o : %.asm
	$(NASM) $(NASM_FLAGS) $*.asm -o $@

$(ELFS): %.elf : %.o $(STD_C_OBJECTS) $(STD_ASM_OBJECTS)
	$(LD) $(LD_FLAGS) -T $(LINKER_SCRIPT) $^ -o $@

$(STRIPS): %.strp : %.elf
	objcopy -I elf32-i386 -O binary $*.elf $@

$(BINS) : %.bin : %.strp
	dd if=/dev/zero of=$@ bs=512 count=128 conv=notrunc
	dd if=$*.strp of=$@ bs=512 count=128 conv=notrunc

$(BIN) : $(BINS)
	cat $^ > $@


.PHONY: all