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

# Build tools and options
NASM = nasm
NASM_FLAGS = -felf32 -g

GCC = gcc
MAIN_FLAGS = -xc -std=c99 -m32 -ffreestanding -no-pie -fno-pie -mno-sse -fno-stack-protector -masm=intel
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Wno-implicit-fallthrough
DEBUG_FLAGS = -O0 -g3 -DDEBUG
RELEASE_FLAGS = -O2 -Werror
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
GCC_FLAGS += $(MODE_FLAGS)

# QEMU
QEMU = qemu-system-i386
QEMU_FLAGS = -cpu pentium2 -m 1g -monitor stdio -device VGA -no-shutdown -no-reboot
QEMU_BOOT_DEVICE = -drive if=floppy,index=0,format=raw,file=boot.img

# maximum kernel size in kB
KERNEL_SIZE_MAX = 20

# =============================================================================
# Tasks

all: kill run-debug

# Incremental tasks
$(BUILD_DIR)/:
	@mkdir -p $@

boot.img: $(BUILD_DIR)/os.bin $(BUILD_DIR)/kernel_size.check | $(BUILD_DIR)/
	@echo -e "\t\e[1mMaking image\e[0m"
	@dd if=$(BUILD_DIR)/os.bin of=boot.img conv=notrunc

$(C_OBJECTS): $(C_SOURCES) $(C_HEADERS) | $(BUILD_DIR)/
	@echo -e "\t\e[1mCompiling\e[0m" $(notdir $*)
	$(GCC) $(GCC_FLAGS) -c $(shell find -L ./$(SOURCE_DIR)/ -iname "$(notdir $*).c") -o $@

$(ASM_OBJECTS): $(ASM_SOURCES) | $(BUILD_DIR)/
	@echo -e "\t\e[1mAssembling\e[0m" $(notdir $*)
	$(NASM) $(NASM_FLAGS) $(shell find -L ./$(SOURCE_DIR)/ -iname "$(notdir $*).asm") -o $@

$(BUILD_DIR)/os.elf: $(ASM_OBJECTS) $(C_OBJECTS) $(LINKER_SCRIPT) | $(BUILD_DIR)/
	@echo -e "\t\e[1mLinking\e[0m"
	@$(GCC) $(BUILD_DIR)/boot.o -E -P -DBUILD_DIR=$(BUILD_DIR) -x c $(LINKER_SCRIPT) > $(BUILD_DIR)/$(LINKER_SCRIPT) 2>$(NULL)
	$(LD) $(LD_FLAGS) -T $(BUILD_DIR)/$(LINKER_SCRIPT) $(BUILD_DIR)/boot.o $(C_OBJECTS) -o $(BUILD_DIR)/os.elf

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/os.elf | $(BUILD_DIR)/
	@echo -e "\t\e[1mBuilding binary from ELF\e[0m"
	objcopy -I elf32-i386 -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin

$(BUILD_DIR)/kernel_size.check: $(BUILD_DIR)/os.bin ./check.sh | $(BUILD_DIR)/
	@echo -e "\t\e[1mChecking kernel size\e[0m"
	@./check.sh $(BUILD_DIR) $(KERNEL_SIZE_MAX)
	touch $@

compile_commands.json: MAKEFLAGS += --no-print-directory
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
	@echo ECHO: $(ASM_OBJECTS)

compile: clean-compile $(C_OBJECTS)
assemble: clean-assemble $(ASM_OBJECTS)
link: clean-link $(BUILD_DIR)/os.elf
bin: clean-bin $(BUILD_DIR)/os.bin
check: $(BUILD_DIR)/kernel_size.check
image: boot.img

clangd: compile_commands.json

clean: clean-compile clean-assemble clean-link clean-bin clean-image clean-check
	rm -rf $(BUILD_DIR) 

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

# ===== BUILD MODES =====
export MODE
export MODE_FLAGS
build: | $(BUILD_DIR)/
ifeq (,$(wildcard $(MODE)))
	$(MAKE) clean
	$(MAKE) $(MODE)
endif
	$(MAKE) boot.img


$(BUILD_DIR)/.RELEASE: | $(BUILD_DIR)/
	touch $@

build-release: MODE = $(BUILD_DIR)/.RELEASE
build-release: MODE_FLAGS += $(RELEASE_FLAGS)
build-release: MAKEFLAGS += --no-print-directory
build-release:
	$(MAKE) build

release: run-release
run-release: build-release
	@echo -e "\t\e[1mRunning release\e[0m"
	$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE)

$(BUILD_DIR)/.DEBUG: | $(BUILD_DIR)/
	touch $@

build-debug: MODE = $(BUILD_DIR)/.DEBUG
build-debug: MODE_FLAGS += $(DEBUG_FLAGS)
build-debug: MAKEFLAGS += --no-print-directory
build-debug:
	$(MAKE) build

test: run-debug
run-debug: build-debug
	@echo -e "\t\e[1mRunning debug\e[0m"
	$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE)

debug: kill
ifeq ("$(wildcard boot.img)","")
	@$(MAKE) build-debug
endif
	@echo -e "\t\e[1mDebugging\e[0m"
	$(QEMU) $(QEMU_FLAGS) $(QEMU_BOOT_DEVICE) -s -S &
	gdb
	@$(MAKE) kill

.PHONY: all test release clean debug compile assemble link check kill clean-compile clean-assemble clean-link clean-bin clean-image clean-check clangd
.PHONY: build-release run-release build-debug run-debug build
