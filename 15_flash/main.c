#include "stm32f4xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

#include "stm32f4xx.h"

#define FLASH_ADDR  0x08020000UL
#define TEST_DATA   0x12345678UL

//const uint32_t sect_addr[8] = {
//	0x08000000,
//	0x08004000,
//	0x08008000,
//	0x0800C000,
//	0x08010000,
//	0x08020000,
//	0x08040000,
//	0x08060000
//}

void flash_test(void) {
	uint32_t result;

	printf("\r\n--- FLASH TEST ---\r\n");

	// Wait for Flash not busy
	while (FLASH->SR & FLASH_SR_BSY);

	// Unlock Flash
	if (FLASH->CR & FLASH_CR_LOCK) {
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}

	if (FLASH->CR & FLASH_CR_LOCK) {
		printf("ERROR: Flash unlock failed!\r\n");
		return;
	}

	printf("Flash unlocked\r\n");

	// Erase Sector 5
	printf("Erasing Sector 5...\r\n");

	FLASH->CR &= ~(FLASH_CR_SNB | FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_CR_SER | (5U << 3);
	FLASH->CR |= FLASH_CR_STRT;

	while (FLASH->SR & FLASH_SR_BSY);

	if (FLASH->SR & (FLASH_SR_PGSERR | FLASH_SR_PGPERR |
				FLASH_SR_PGAERR | FLASH_SR_WRPERR |
				FLASH_SR_OPERR)) {
		printf("ERROR: Erase failed, SR = 0x%08lX\r\n",
				(unsigned long)FLASH->SR);
		FLASH->SR |= FLASH_SR_PGSERR | FLASH_SR_PGPERR |
			FLASH_SR_PGAERR | FLASH_SR_WRPERR |
			FLASH_SR_OPERR;
		FLASH->CR &= ~FLASH_CR_SER;
		FLASH->CR |= FLASH_CR_LOCK;
		return;
	}

	FLASH->CR &= ~FLASH_CR_SER;

	printf("Sector erased\r\n");

	// Write 32-bit data
	printf("Writing 0x%08lX to 0x%08lX...\r\n",
			(unsigned long)TEST_DATA,
			(unsigned long)FLASH_ADDR);

	FLASH->CR |= FLASH_CR_PG | FLASH_CR_PSIZE_1;

	*(volatile uint32_t *)FLASH_ADDR = TEST_DATA;

	while (FLASH->SR & FLASH_SR_BSY);

	FLASH->CR &= ~FLASH_CR_PG;

	printf("Write complete\r\n");

	// Read data
	result = *(volatile uint32_t *)FLASH_ADDR;

	printf("Read from Flash: 0x%08lX\r\n",
			(unsigned long)result);

	// Verify
	if (result == TEST_DATA) {
		printf("PASS: Data verified!\r\n");
	} else {
		printf("FAIL: Expected 0x%08lX, got 0x%08lX\r\n",
				(unsigned long)TEST_DATA,
				(unsigned long)result);
	}

	// Lock Flash
	FLASH->CR |= FLASH_CR_LOCK;

	printf("Flash locked\r\n");
	printf("--- TEST FINISHED ---\r\n");
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
