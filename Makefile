################################################################################
# Makefile for Raspberry Pi 3B+ AArch64 Bare-Metal Project
################################################################################

################################################################################
# Toolchain
################################################################################

GCC_TOOLCHAIN := aarch64-none-elf

AS64      := $(GCC_TOOLCHAIN)-as
GCC64     := $(GCC_TOOLCHAIN)-gcc
LD64      := $(GCC_TOOLCHAIN)-ld
OBJCOPY64 := $(GCC_TOOLCHAIN)-objcopy
OBJDUMP64 := $(GCC_TOOLCHAIN)-objdump

################################################################################
# Processor options
################################################################################

CPU  = cortex-a53
ARCH = armv8-a

################################################################################
# Directories
################################################################################

BOOT_DIR    := boot
DRIVER_DIR  := drivers/uart
MMU_DIR     := mmu
LIB_DIR     := lib
INC_DIR     := include
LINKER_DIR  := linker
SRC_DIR     := src
BUILD_DIR   := build

################################################################################
# Dhrystone iterations count
################################################################################

ifndef ITERATIONS
    DHRYSTONE_ITER := 100000000
else
    DHRYSTONE_ITER := $(ITERATIONS)
endif

################################################################################
# Assembler, compiler, and linker options
################################################################################

ASM_OPTS  = -march=$(ARCH) -I$(INC_DIR)

# Dhrystone specific C compiler's flags
DHRY_CC_OPTS = -Wno-implicit-function-declaration -Wno-implicit-int \
               -Wno-return-type -Wno-format -Wno-pointer-to-int-cast \
               -O1 --include "string.h" -DMSC_CLOCK -DITERATIONS=$(DHRYSTONE_ITER) \
               -fno-inline -fno-unroll-loops -fno-tree-vectorize -fno-builtin-printf \
               -fno-tree-loop-vectorize -fno-tree-slp-vectorize \
               -mstrict-align

CC_OPTS   = -march=$(ARCH) -mtune=$(CPU) -c -DGCC \
            $(DHRY_CC_OPTS) \
            -Wno-builtin-declaration-mismatch -g -I$(INC_DIR)

LINK_OPTS = -Wl,-T $(LINKER_DIR)/linker.ld

################################################################################
# Source files
################################################################################

# Assembly sources
ASM_SRCS :=

# C sources
C_SRCS   := $(SRC_DIR)/dhry_1.c \
            $(SRC_DIR)/dhry_2.c \
            $(LIB_DIR)/crt0.c \
            $(LIB_DIR)/clock.c \
            $(DRIVER_DIR)/uart.c \
            $(DRIVER_DIR)/output_char.c \
            $(LIB_DIR)/retarget.c \
            $(BOOT_DIR)/stackheap.c \
            $(BOOT_DIR)/init.c \
            $(BOOT_DIR)/vectors.c \
            $(MMU_DIR)/pagetables.c

################################################################################
# Object files (in build directory)
################################################################################

ASM_OBJS := $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SRCS))
C_OBJS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))

################################################################################
# Target
################################################################################

TARGET := $(BUILD_DIR)/dhrystone

.PHONY: build clean

build: $(TARGET).disasm $(TARGET).elf

################################################################################
# Build rules
################################################################################

# Create build subdirectories as needed
$(BUILD_DIR)/%/:
	@mkdir -p $@

# Assemble .s files
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo " [ASM ] $<"
	@$(AS64) $(ASM_OPTS) $< -o $@

# Compile .c files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo " [CC  ] $<"
	@$(GCC64) $(CC_OPTS) $< -o $@

# Link
$(TARGET).elf: $(ASM_OBJS) $(C_OBJS)
	@echo " [LINK] $@"
	@$(GCC64) $(LINK_OPTS) $^ -o $@

# Disassemble
$(TARGET).disasm: $(TARGET).elf
	@echo " [DISASM] $@"
	@$(OBJDUMP64) -d $< > $@

################################################################################
# Clean
################################################################################

clean:
	@rm -rf $(BUILD_DIR)
