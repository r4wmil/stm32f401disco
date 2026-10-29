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
		led_on(2);
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

int state_read_packet(uint8_t b) {
	if (!read_key(b)) { return 0; }
	if (pr.buftop < 8) { return 0; }
	pr.len  = bs_getu32l(pr.buf + 0);
	if (pr.buftop - 8 > pr.len) { return 2; }
	pr.hash = bs_getu32l(pr.buf + 4);
	if (pr.buftop < 8 + pr.len) { return 0; }
	uint32_t new_hash = crc_32(pr.buf + 8, pr.len);
	if (pr.hash == new_hash) {
		usart2_send(0xFA);
		led_on(0);
		return 1;
	}
	bufoff();
	return 2;
}

void handle(uint8_t b) {
	bufadd(b);
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_READ_PACKET:
		int code = state_read_packet(b);
		if (code == 2) {
			// TODO: shared codes
			usart2_send(0xE4);
			state = STATE_READ_PACKET;
		}
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
	led_on(2);
	while (1) {
		__NOP();
	}
}
