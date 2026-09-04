EXAMPLE=1_blink

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
STFLASH = st-flash

CFLAGS = -mcpu=cortex-m4 -mthumb -specs=nosys.specs -DSTM32F401xC
CFLAGS += -I. -I/usr/arm-none-eabi/include
CFLAGS += -I./Drivers/CMSIS/Core/Include -I./Drivers/CMSIS/STM32F4xx/Include
CFLAGS += -I$(EXAMPLE)

STARTUP = ./startup_stm32f401xc.s

SRCS = $(EXAMPLE)/main.c
SRCS += ./system_stm32f4xx.c

LDFLAGS = -T ./STM32F401CCUX_FLASH.ld

OUT_DIR = out
ELF = $(OUT_DIR)/binary.elf
BIN = $(OUT_DIR)/binary.bin

# --- RTOS NEEDS SPECIAL LINKING ---
ifeq ($(EXAMPLE),12_rtos)
	STARTUP = ./startup_freertos.s
	CFLAGS += -g3
	CFLAGS += -IFreeRTOS/include
	CFLAGS += -IFreeRTOS/portable/GCC/ARM_CM4F
	CFLAGS += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
	SRCS += FreeRTOS/tasks.c \
		FreeRTOS/queue.c \
		FreeRTOS/list.c \
		FreeRTOS/portable/GCC/ARM_CM4F/port.c \
		FreeRTOS/portable/MemMang/heap_4.c
endif

# --- BOOTLOADER NEEDS SPECIAL BUILDING ---
ifeq ($(EXAMPLE),13_bootloader)

BOOT_LD  = $(EXAMPLE)/bootloader.ld
APP1_LD  = $(EXAMPLE)/app1.ld
APP2_LD  = $(EXAMPLE)/app2.ld

BOOT_ELF = $(OUT_DIR)/bootloader.elf
BOOT_BIN = $(OUT_DIR)/bootloader.bin
APP1_ELF = $(OUT_DIR)/app1.elf
APP1_BIN = $(OUT_DIR)/app1.bin
APP2_ELF = $(OUT_DIR)/app2.elf
APP2_BIN = $(OUT_DIR)/app2.bin

all: clean $(OUT_DIR) $(BOOT_BIN) $(APP1_BIN) $(APP2_BIN)
	$(STFLASH) --connect-under-reset write $(BOOT_BIN) 0x08000000
	$(STFLASH) --connect-under-reset write $(APP1_BIN) 0x08008000
	$(STFLASH) --connect-under-reset write $(APP2_BIN) 0x08020000

$(BOOT_ELF):
	$(CC) $(CFLAGS) $(EXAMPLE)/bootloader.c system_stm32f4xx.c $(STARTUP) \
		-o $@ -T $(BOOT_LD)

$(BOOT_BIN): $(BOOT_ELF)
	$(OBJCOPY) -O binary $< $@

$(APP1_ELF):
	$(CC) $(CFLAGS) $(EXAMPLE)/app1.c system_stm32f4xx.c $(STARTUP) \
		-o $@ -T $(APP1_LD)

$(APP2_ELF):
	$(CC) $(CFLAGS) $(EXAMPLE)/app2.c system_stm32f4xx.c $(STARTUP) \
		-o $@ -T $(APP2_LD)

$(APP1_BIN): $(APP1_ELF)
	$(OBJCOPY) -O binary $< $@

$(APP2_BIN): $(APP2_ELF)
	$(OBJCOPY) -O binary $< $@

else
# --- DEFAULT PROJECT BUILD ---
SRCS += $(STARTUP)
all: clean $(OUT_DIR) $(BIN)
	$(STFLASH) --connect-under-reset write $(BIN) 0x08000000

$(ELF): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

endif

# --- GENERAL PURPOSE TARGETS ---

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean
