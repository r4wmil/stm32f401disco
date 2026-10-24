#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

#define CRC_IMPLEMENTATION
#include "crc.h"
#undef  CRC_IMPLEMENTATION

#define BINSER_IMPLEMENTATION
#include "binser.h"
#undef  BINSER_IMPLEMENTATION

enum {
	STATE_PENDING = 0,
	STATE_WAITING
};

int state = STATE_PENDING;

void state_pending(uint8_t b) {
	static bool sent = 0;
	if (!sent && b == 0xF1) {
		usart2_send(0x1E);
		GPIOD->ODR &= ~(1U << LEDS + 2);
		sent = 1;
		state = STATE_WAITING;
	}
}

void precv(uint8_t b) {
	static bool is_first = 1;
	if (is_first && b == 0x01) {
		is_first = 0;
		return;
	}
	static uint8_t buf[1024];
	static size_t buftop = 0;
	if (buftop < 1024) {
		buf[buftop++] = b;
	}
	// Parse
	if (buftop < 8) { return; }
	uint32_t len  = bs_getu32l(buf + 0);
	uint32_t hash = bs_getu32l(buf + 4);
	if (buftop < 8 + len) { return; }
	uint32_t new_hash = crc_32(buf + 8, len);
	if (hash == new_hash) {
		usart2_send(0xFA);
		GPIOD->ODR |=  (1U << LEDS + 0);
	}
	//GPIOD->ODR |=  (1U << LEDS + 1);
	//uint8_t lenb[4];
	//lenb[0] = b;
	//for (int i = 1; i < 4; i++) lenb[i] = usart2_recv();
	//uint32_t len = bs_getu32l(lenb);
	//uint8_t hashb[4];
	//for (int i = 0; i < 4; i++) hashb[i] = usart2_recv();
	//uint32_t hash = bs_getu32l(hashb);
	//uint8_t buf[1024];
	//for (int i = 0; i < len; i++) { buf[i] = usart2_recv(); }
	//uint32_t new_hash = crc_32(buf, len);
	//if (hash == new_hash) {
	//	usart2_send(0xFA);
	//	GPIOD->ODR |=  (1U << LEDS + 0);
	//}
}

void handle(uint8_t b) {
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_WAITING:
		precv(b);
		break;
	}
}

void USART2_IRQHandler(void) {
	uint8_t b = (uint8_t)USART2->DR;
	handle(b);
}

int main(void) {
	leds_init();
	pll_init();
	usart2_init();
	NVIC_EnableIRQ(USART2_IRQn);
	GPIOD->ODR |=  (1U << LEDS + 2);
	while (1) {
		__NOP();
	}
}
