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
	STATE_READ_HEADER,
	STATE_READ_PAYLOAD
};

int state = STATE_PENDING;

void state_pending(uint8_t b) {
	static bool sent = 0;
	if (!sent && b == 0xF1) {
		usart2_send(0x1E);
		GPIOD->ODR &= ~(1U << LEDS + 2);
		sent = 1;
		state = STATE_READ_HEADER;
	}
}

typedef struct page_reader {
	bool bufen;
	uint8_t buf[1024];
	size_t buftop;
	uint32_t len;
	uint32_t hash;
} page_reader;

page_reader pr = {0};


void state_read_header(uint8_t b) {
	if (!pr.bufen && b == 0x01) {
		pr.bufen = 1;
		return;
	}
	if (pr.buftop < 8) { return; }
	pr.len  = bs_getu32l(pr.buf + 0);
	pr.hash = bs_getu32l(pr.buf + 4);
	state = STATE_READ_PAYLOAD;
}

void state_read_payload(uint8_t b) {
	if (pr.buftop < 8 + pr.len) { return; }
	uint32_t new_hash = crc_32(pr.buf + 8, pr.len);
	if (pr.hash == new_hash) {
		usart2_send(0xFA);
		GPIOD->ODR |=  (1U << LEDS + 0);
	}
}

void bufadd(uint8_t b) {
	if (!pr.bufen) { return; }
	if (pr.buftop < 1024) {
		pr.buf[pr.buftop++] = b;
	}
}

void handle(uint8_t b) {
	bufadd(b);
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_READ_HEADER:
		state_read_header(b);
		break;
	case STATE_READ_PAYLOAD:
		state_read_payload(b);
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
