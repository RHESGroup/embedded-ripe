#-----------------------------------------------------------------------------------------------------------------------
# Makefile for the embedded-ripe benchmark
#
# Supported platforms:
# - qemu (netduinoplus2)
# - discovery (stm32f4-discovery)
# - nucleo (nucleo-144)
#
# The makefile must be invoked with a mandatory target platform
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Common output variables
#-----------------------------------------------------------------------------------------------------------------------

OUTPUT_DIR := ./output
IMAGE_ELF := ripe.elf
IMAGE_BIN := ripe.bin
IMAGE_MAP := ripe.map

#-----------------------------------------------------------------------------------------------------------------------
# Target dependent variables
#-----------------------------------------------------------------------------------------------------------------------

qemu: BOARD_DIR = qemu
qemu: LINKER_SCRIPT_NAME = stm32f4xx.ld
qemu: STARTUP_SCRIPT_NAME = startup_gcc.c

discovery: BOARD_DIR = discovery
discovery: LINKER_SCRIPT_NAME = stm32f4xx.ld
discovery: STARTUP_SCRIPT_NAME = startup_stm32f4.s

nucleo: BOARD_DIR = nucleo
nucleo: LINKER_SCRIPT_NAME = stm32f4xx.ld
nucleo: STARTUP_SCRIPT_NAME = startup_stm32f4.s

#-----------------------------------------------------------------------------------------------------------------------
# Compiler setup and flags
#-----------------------------------------------------------------------------------------------------------------------

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
SIZE = arm-none-eabi-size
OBJCP = arm-none-eabi-objcopy

LINKER = ./../boards/$(BOARD_DIR)/$(LINKER_SCRIPT_NAME) # 	Specify the used linker script

CFLAGS += $(INCLUDE_DIRS) # 								Specify include directories
CFLAGS += -nostartfiles	# 									Do not use the standard system startup files
CFLAGS += -ffreestanding # 									Direct the compiler to not assumer std functions exists
CFLAGS += -mthumb # 										Generate thumb code
CFLAGS += -mcpu=cortex-m4 # 								Specify the target processor name
CFLAGS += -Wall -Wextra	# 									Enable all and extra warnings during compilation
CFLAGS += -g3 -O0 # 										Enable maximum debug info and disable optimization
CFLAGS += -ffunction-sections # 							Generate a separate ELF section for each function
CFLAGS += -fdata-sections # 								Place each data item in a separate section
CFLAGS += -mfloat-abi=softfp # 								Allow use hw FPU but with soft ABI convention	
CFLAGS += -mfpu=fpv4-sp-d16	# 								Specify the hw FPU available on target
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT $@	# 				Allow preprocessor to generate dependencies files

LDFLAGS += -Xlinker --gc-sections # 						Enable garbage collection of unused sections
LDFLAGS += -Xlinker -T $(LINKER) # 							Specify the linker script
LDFLAGS += -Xlinker -Map=$(OUTPUT_DIR)/$(IMAGE_MAP) # 		Generate a memory map after linking
LDFLAGS += -specs=nano.specs # 								Link with newlib-nano
LDFLAGS += -specs=nosys.specs # 							No syscall linking
LDFLAGS += -specs=rdimon.specs # 							Link with libgloss (low-level I/O support)

#--------------------------------------------------- 
# FreeRTOS kernel build
#---------------------------------------------------

# The directory that contains the /source and /demo sub directories.
FREERTOS_ROOT = ./../../..

KERNEL_DIR = $(FREERTOS_ROOT)/Source
KERNEL_PORT_DIR += $(KERNEL_DIR)/portable/GCC/ARM_CM4F

INCLUDE_DIRS += -I$(KERNEL_PORT_DIR)
INCLUDE_DIRS += -I$(KERNEL_DIR)/include

VPATH += $(KERNEL_DIR) 
VPATH += $(KERNEL_PORT_DIR)
VPATH += $(KERNEL_DIR)/portable/MemMang

SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/timers.c
SOURCE_FILES += $(KERNEL_DIR)/event_groups.c
SOURCE_FILES += $(KERNEL_DIR)/stream_buffer.c
SOURCE_FILES += $(KERNEL_DIR)/portable/MemMang/heap_4.c
SOURCE_FILES += $(KERNEL_DIR)/portable/GCC/ARM_CM4F/port.c

#--------------------------------------------------- 
# FreeRTOS common demo files build
#---------------------------------------------------

DEMO_ROOT = $(FREERTOS_ROOT)/Demo

COMMON_DEMO_FILES = $(DEMO_ROOT)/Common/Minimal

INCLUDE_DIRS += -I$(DEMO_ROOT)/Common/include

VPATH += $(COMMON_DEMO_FILES)

SOURCE_FILES += $(COMMON_DEMO_FILES)/IntQueue.c
SOURCE_FILES += $(COMMON_DEMO_FILES)/IntQueueTimer.c

#--------------------------------------------------- 
# Current demo (embedded-ripe) build
#---------------------------------------------------

DEMO_PROJECT = $(DEMO_ROOT)/CORTEX_M4F_STM32_QEMU_GCC

VPATH += $(DEMO_PROJECT)

INCLUDE_DIRS += -I$(DEMO_PROJECT)
INCLUDE_DIRS += -I$(DEMO_PROJECT)/CMSIS
INCLUDE_DIRS += -I$(DEMO_PROJECT)/boards/$(BOARD_DIR)

SOURCE_FILES += $(DEMO_PROJECT)/main.c
SOURCE_FILES += ./printf-stdarg.c

#--------------------------------------------------- 
# Object file listing and dependencies
#---------------------------------------------------

OBJS = $(SOURCE_FILES:%.c=%.o)
OBJS_NO_PATH = $(notdir $(OBJS))
OBJS_OUTPUT = $(OBJS_NO_PATH:%.o=$(OUTPUT_DIR)/%.o)
OBJS_OUTPUT += $(OUTPUT_DIR)/startup.o

DEP_FILES := $(SOURCE_FILES:%.c=$(OUTPUT_DIR)/%.d)
DEP_FILES_NO_PATH = $(notdir $(DEP_FILES))
DEP_OUTPUT = $(DEP_FILES_NO_PATH:%.d=$(OUTPUT_DIR)/%.d)

#--------------------------------------------------- 
# Rules
#---------------------------------------------------

all:
	@echo "You have to specify a target architecture platform among [qemu/discovery/nucleo]"

%.o : %.c
$(OUTPUT_DIR)/%.o : %.c $(OUTPUT_DIR)/%.d Makefile
	@echo "[CC]" $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/startup.o: $(DEMO_PROJECT)/boards/$(BOARD_DIR)/$(STARTUP_SCRIPT_NAME)
	@echo "[CC]" $(DEMO_PROJECT)/boards/$(BOARD_DIR)/$(STARTUP_SCRIPT_NAME)
	@$(CC) $(CFLAGS) -c $(DEMO_PROJECT)/boards/$(BOARD_DIR)/$(STARTUP_SCRIPT_NAME) -o $@

$(OUTPUT_DIR)/$(IMAGE_ELF): $(LINKER) $(OBJS_OUTPUT) Makefile
	@for OB in $(OBJS_OUTPUT); do echo "[LD]" $$OB; done
	@$(LD) $(OBJS_OUTPUT) $(CFLAGS) $(LDFLAGS) -o $(OUTPUT_DIR)/$(IMAGE_ELF)
	@$(SIZE) $(OUTPUT_DIR)/$(IMAGE_ELF)	
	@echo "[CP]" $(OUTPUT_DIR)/$(IMAGE_BIN)
	@$(OBJCP) -O binary $(OUTPUT_DIR)/$(IMAGE_ELF) $(OUTPUT_DIR)/$(IMAGE_BIN)

$(DEP_OUTPUT):
include $(wildcard $(DEP_OUTPUT))

#--------------------------------------------------- 
# Targets
#---------------------------------------------------

qemu: $(OUTPUT_DIR)/$(IMAGE_ELF)

discovery: $(OUTPUT_DIR)/$(IMAGE_ELF)

nucleo: $(OUTPUT_DIR)/$(IMAGE_ELF)

clean:
	rm -f $(OUTPUT_DIR)/$(IMAGE_ELF) $(OUTPUT_DIR)/$(IMAGE_BIN) $(OUTPUT_DIR)/$(IMAGE_MAP) $(OUTPUT_DIR)/*.o $(OUTPUT_DIR)/*.d

# Use "make print-[VARIABLE_NAME] to print the value of a variable generated by this makefile.
print-%  : ; @echo $* = $($*)

discovery-run, nucleo-run:
	@st-flash write $(OUTPUT_DIR)/$(IMAGE_BIN) 0x8000000

discovery-debug, nucleo-debug:
	@st-util
	
qemu-run:
	qemu-system-arm -M netduinoplus2 -kernel $(OUTPUT_DIR)/$(IMAGE_ELF) -monitor none -nographic -serial /dev/null -serial stdio

qemu-debug:
	qemu-system-arm -M netduinoplus2 -kernel $(OUTPUT_DIR)/$(IMAGE_ELF) -monitor none -nographic -serial /dev/null -serial stdio -s -S


.PHONY: all qemu discovery nucleo clean print-% discovery-run, discovery-debug nucleo-run, nucleo-debug, qemu-run, qemu-debug

