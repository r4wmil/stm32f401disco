#include "stm32f4xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

#include "stm32f4xx.h"

volatile uint8_t* sector[] = {
	(volatile uint8_t*)0x08000000UL,
	(volatile uint8_t*)0x08004000UL,
	(volatile uint8_t*)0x08008000UL,
	(volatile uint8_t*)0x0800C000UL,
	(volatile uint8_t*)0x08010000UL,
	(volatile uint8_t*)0x08020000UL,
	(volatile uint8_t*)0x08040000UL,
	(volatile uint8_t*)0x08060000UL
};

bool flash_unlock() {
	if (FLASH->CR & FLASH_CR_LOCK) {
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
	return !(FLASH->CR & FLASH_CR_LOCK);
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

bool flash_busy() {
	return (FLASH->SR & FLASH_SR_BSY) > 0;
}

void flash_wait() {
	while (flash_busy());
}

void flash_sector_erase(size_t s) {
	FLASH->CR &= ~(FLASH_CR_SNB | FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_CR_SER | (4U << FLASH_CR_SNB_Pos);
	FLASH->CR |= FLASH_CR_STRT;
}

void flash_test(void) {
	uint8_t buf[5] = {0x12,0x34,0x56,0x78,0x9A};
	printf("flash_unlock()=%d\r\n", flash_unlock());
	flash_sector_erase(4);
	flash_wait();
	flash_begin();
	for (size_t i = 0; i < 5; i++) {
		sector[4][i] = buf[i];
	}
	flash_wait();
	flash_end();
	for (size_t i = 0; i < 5; i++) {
		printf("sector[4][%zu]=%02X\r\n", i, sector[4][i]);
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
