#include "stm32f4xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

#include "stm32f4xx.h"

#define FLASH_ADDR     0x08000000UL
#define FLASH_SECTOR4  0x08010000UL

void flash_unlock() {
	if (FLASH->CR & FLASH_CR_LOCK) {
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
}

void flash_lock() {
	FLASH->CR |= FLASH_CR_LOCK;
}

void flash_set_word_x8() {
	FLASH->CR &= ~FLASH_CR_PSIZE;
}

void flash_begin() {
	FLASH->CR |= FLASH_CR_PG;
}

void flash_end() {
	FLASH->CR &= ~FLASH_CR_PG;
}

void flash_wait() {
	while (FLASH->SR & FLASH_SR_BSY);
}

volatile uint8_t* flash = FLASH_ADDR;
volatile uint8_t* sector4 = FLASH_SECTOR4;

void flash_sector4_erase() {
	FLASH->CR &= ~(FLASH_CR_SNB | FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_CR_SER | (4U << FLASH_CR_SNB_Pos);
	FLASH->CR |= FLASH_CR_STRT;
}

void flash_test(void) {
	uint8_t buf[5] = {0x12,0x34,0x56,0x78,0x9A};
	flash_unlock();
	flash_sector4_erase();
	flash_wait();
	flash_begin();
	for (size_t i = 0; i < 5; i++) {
		sector4[i] = buf[i];
	}
	flash_wait();
	flash_end();
	for (size_t i = 0; i < 5; i++) {
		printf("sector4[%zu]=%02X\r\n", i, sector4[i]);
	}
	flash_lock();
}

int main(void) {
	leds_init();
	led_on(0);
	pll_init();
	usart2_init();
	printf("\033[2J\033[H");
	printf("hello!\r\n");
	flash_test();
	while (1) {
		__NOP();
	}
}
