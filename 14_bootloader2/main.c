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
	STATE_READ_PACKET
};

int state = STATE_PENDING;

void state_pending(uint8_t b) {
	static bool sent = 0;
	if (!sent && b == 0xF1) {
		usart2_send(0x1E);
		GPIOD->ODR &= ~(1U << LEDS + 2);
		sent = 1;
		state = STATE_READ_PACKET;
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

void bufadd(uint8_t b) {
	if (pr.bufen && pr.buftop < 1024) {
		pr.buf[pr.buftop] = b;
		pr.buftop++;
	}
}

void bufon(uint8_t b) { pr.bufen = 1; }

void bufoff() { pr.bufen = 0; }

bool read_key(uint8_t b) {
	static uint8_t key[4] = {0x12,0xEF,0x34,0xCD};
	static size_t top = 0;
	if (top == 4) { return 1; }
	if (key[top] == b) {
		top++;
	} else {
		top = 0;
	}
	if (top == 4) {
		bufon(b);
		return 1;
	}
	return 0;
}

void state_read_packet(uint8_t b) {
	if (!read_key(b)) { return; }
	if (pr.buftop < 8) { return; }
	pr.len  = bs_getu32l(pr.buf + 0);
	//TODO: check buf max size gracefully
	if (pr.len > 1024) pr.len = 1024;
	//for (size_t i = 0; i < 4; i++) usart2_send(pr.buf[i]);
	pr.hash = bs_getu32l(pr.buf + 4);
	if (pr.buftop < 8 + pr.len) { return; }
	uint32_t new_hash = crc_32(pr.buf + 8, pr.len);
	if (pr.hash == new_hash) {
		usart2_send(0xFA);
		GPIOD->ODR |=  (1U << LEDS + 0);
	}
	bufoff();
	state = STATE_PENDING;
}

void handle(uint8_t b) {
	bufadd(b);
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_READ_PACKET:
		state_read_packet(b);
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
