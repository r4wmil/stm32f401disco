# TODO: building without need to clear out/* by hand

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

SRCS += $(STARTUP)

all: $(OUT_DIR) $(BIN)
	$(STFLASH) --connect-under-reset write $(BIN) 0x08000000

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(ELF): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean
