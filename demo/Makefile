# Makefile for S32K3X8EVB + FreeRTOS (Cortex-M7) on QEMU

# The directory that contains FreeRTOS source code
FREERTOS_ROOT := ./../freertos/FreeRTOS

# Demo code
DEMO_PROJECT := .

# QEMU build directory
BUILD_DIR := ../build

# Where to store all the generated files (objects, elf and map)
OUTPUT_DIR := ./Output

DEMO_NAME := demo
ELF := $(OUTPUT_DIR)/$(DEMO_NAME).elf
MAP := $(OUTPUT_DIR)/$(DEMO_NAME).map

# Compiler toolchain
CC := arm-none-eabi-gcc
LD := arm-none-eabi-gcc
SIZE := arm-none-eabi-size

# Target embedded board and CPU
MACHINE := s32k3x8evb
CPU := cortex-m7

# Use -s to connect to gdb port 1234 and -S to wait before executing
QEMU_FLAGS = -s -S 

# FreeRTOS kernel
KERNEL_DIR := $(FREERTOS_ROOT)/Source
KERNEL_PORT_DIR := $(KERNEL_DIR)/portable/GCC/ARM_CM7/r0p1

INCLUDE_DIRS = -I$(KERNEL_DIR)/include -I$(KERNEL_PORT_DIR)
INCLUDE_DIRS += -I$(DEMO_PROJECT)

VPATH += $(KERNEL_DIR)
VPATH += $(KERNEL_PORT_DIR)
VPATH += $(KERNEL_DIR)/portable/MemMang
VPATH += $(DEMO_PROJECT)

# Include paths. See INCLUDE_DIRS
CFLAGS = $(INCLUDE_DIRS)

# Don't include standard libraries
CFLAGS += -ffreestanding

# Target ARM Cortex-M7 processor
CFLAGS += -mcpu=$(CPU)
# Use the thumb 16 bits instruction set. Required for cortex-M
CFLAGS += -mthumb

# Enable hardware floating point support required by FreeRTOS port for Cortex-M7
CFLAGS += -mfpu=fpv5-d16 -mfloat-abi=hard

# Print all the most common warnings
CFLAGS += -Wall

# Print extra warnings, not included in -Wall
CFLAGS += -Wextra

# Print warnings about variable shadowing (var name = outer scope var name)
CFLAGS += -Wshadow

# Include debug information (-g) with maximum level of detail (3)
CFLAGS += -g3

# Optimize (-O) the size (s) of the generated executable 
CFLAGS += -Os

# Place each function in a dedicated section (see -gc-sections in LDFLAGS)
CFLAGS += -ffunction-sections

# Place each variable in a dedicated section (see -gc-sections in LDFLAGS)
CFLAGS += -fdata-sections

# Specify the linker script
LDFLAGS = -T ./linker.ld

# Don't use the standard start-up files
LDFLAGS += -nostartfiles

# Optimize compilation to reduce memory
LDFLAGS += -specs=nano.specs

# Optimize compilation targeting a system without any operating system
LDFLAGS += -specs=nosys.specs

# Generate map file with memory layout. Pass option to the linker (-Xlinker)
LDFLAGS += -Xlinker -Map=$(MAP)

# Remove unused sections. Used with -ffunction-sections and -fdata-sections
# optimized memory. Pass option to the linker (-Xlinker)
LDFLAGS += -Xlinker --gc-sections

# Link specifically for cortex M7 and thumb instruction set
LDFLAGS += -mcpu=$(CPU) -mthumb

# Enable hardware floating point support to match FreeRTOS ARM_CM7 port requirements
LDFLAGS += -mfpu=fpv5-d16 -mfloat-abi=hard

# Kernel files
SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/stream_buffer.c
SOURCE_FILES += $(KERNEL_DIR)/event_groups.c
SOURCE_FILES += $(KERNEL_DIR)/portable/MemMang/heap_4.c
SOURCE_FILES += $(KERNEL_PORT_DIR)/port.c

# Demo files
SOURCE_FILES += $(DEMO_PROJECT)/main.c
SOURCE_FILES += $(DEMO_PROJECT)/uart.c
SOURCE_FILES += $(DEMO_PROJECT)/gpio.c
SOURCE_FILES += $(DEMO_PROJECT)/mpu.c

# Start-up code
SOURCE_FILES += ./startup.c

# Create list of object files with the same names of the sources
OBJS = $(SOURCE_FILES:%.c=%.o)

# Remove path from object filename
OBJS_NOPATH = $(notdir $(OBJS))

# Prepend output dir to object filenames
OBJS_OUTPUT = $(OBJS_NOPATH:%.o=$(OUTPUT_DIR)/%.o)

all: $(ELF)

$(ELF): $(OBJS_OUTPUT) ./linker.ld Makefile
	echo "\n\n--- Final linking ---\n"
	$(LD) $(LDFLAGS) $(OBJS_OUTPUT) -o $(ELF)
	$(SIZE) $(ELF)

$(OUTPUT_DIR)/%.o : %.c  Makefile | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

cleanobj:
	rm -f $(OUTPUT_DIR)/*.o

clean:
	rm -rf $(ELF) $(MAP) $(OUTPUT_DIR)/*.o $(OUTPUT_DIR)

qemu_start:
	$(BUILD_DIR)/qemu-system-arm -machine $(MACHINE) -cpu $(CPU) -kernel \
	../test-1.elf -monitor none -nographic -serial stdio

qemu_debug:
	$(BUILD_DIR)/qemu-system-arm -machine $(MACHINE) -cpu $(CPU) -kernel \
	$(ELF) -monitor none -nographic -serial stdio $(QEMU_FLAGS)

gdb_start:
	gdb-multiarch $(ELF)
