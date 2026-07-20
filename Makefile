CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
STFLASH = st-flash

CFLAGS = -mcpu=cortex-m4 -mthumb -specs=nosys.specs -DSTM32F401xC
CFLAGS += -I. -I/usr/arm-none-eabi/include
CFLAGS += -I./CMSIS/Core/Include -I./CMSIS/STM32F4xx/Include

SRCS = 1_blink/main.c
SRCS += ./startup_stm32f401xc.s
SRCS += ./system_stm32f4xx.c

LDFLAGS = -T ./STM32F401CCUX_FLASH.ld

OUT_DIR = out
ELF = $(OUT_DIR)/binary.elf
BIN = $(OUT_DIR)/binary.bin

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
