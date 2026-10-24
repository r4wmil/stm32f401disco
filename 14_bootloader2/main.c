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
	GPIOD->ODR |=  (1U << LEDS + 1);
	uint8_t lenb[4];
	lenb[0] = b;
	for (int i = 1; i < 4; i++) lenb[i] = usart2_recv();
	uint32_t len = bs_getu32l(lenb);
	uint8_t hashb[4];
	for (int i = 0; i < 4; i++) hashb[i] = usart2_recv();
	uint32_t hash = bs_getu32l(hashb);
	uint8_t buf[1024];
	for (int i = 0; i < len; i++) { buf[i] = usart2_recv(); }
	uint32_t new_hash = crc_32(buf, len);
	if (hash == new_hash) {
		usart2_send(0xFA);
		GPIOD->ODR |=  (1U << LEDS + 0);
	}
}

void state_waiting(uint8_t b) {
	if (b) {
		precv(b);
		state = STATE_PENDING;
		//usart2_send(b);
		//if (b == 0x12) GPIOD->ODR |=  (1U << LEDS + 1);
	}
}

void handle(uint8_t b) {
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_WAITING:
		state_waiting(b);
		break;
	}
}

int main(void) {
	leds_init();
	pll_init();
	usart2_init();
	GPIOD->ODR |=  (1U << LEDS + 2);
	while (1) {
		uint8_t b = usart2_recv();
		handle(b);
	}
}
