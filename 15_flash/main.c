#include "stm32f4xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

#include "stm32f4xx.h"

void flash_test(void) {
	uint8_t buf[5] = {0x12,0x34,0x56,0x78,0x9A};
	printf("flash_unlock()=%d\r\n", flash_unlock());
	flash_set_word_x8();
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
