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

#include "codes.h"

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

int state_read_packet(uint8_t b) {
	enum { WAIT_SOH, READ_LEN, READ_PAYLOAD, READ_CRC };
	static int state = WAIT_SOH;
	static uint32_t pos;

	switch (state) {
	case WAIT_SOH:
		if (b != HOST_SOH) return 0;
		pr.buftop = 0;
		pos = 0;
		pr.bufen = true;
		state = READ_LEN;
		return 0;
	case READ_LEN:
		if (++pos < 4) return 0;
		pos = 0;
		pr.len = bs_getu32l(pr.buf);
		if (pr.len > sizeof(pr.buf) - 8) {
			state = WAIT_SOH;
			pr.bufen = false;
			return 2;
		}
		if (pr.len == 0) { return 2; }
		state = READ_PAYLOAD;
		return 0;
	case READ_PAYLOAD:
		if (++pos < pr.len) return 0;
		pos = 0;
		state = READ_CRC;
		return 0;
	case READ_CRC:
		if (++pos < 4) return 0;
		pr.hash = bs_getu32l(pr.buf + pr.buftop - pos);
		pos = 0;
		state = WAIT_SOH;
		pr.bufen = false;
		return pr.hash == crc_32(pr.buf, pr.len + 4) ? 1 : 2;
	}

	return 0;
}

typedef void (*pkt_recv_fn)(uint8_t*, size_t);

void handle(uint8_t b, pkt_recv_fn prf) {
	bufadd(b);
	int code = state_read_packet(b);
	if (code == 1) {
		usart2_send(CLIENT_ACK);
		prf(pr.buf + 8, pr.len);
	} else if (code == 2) {
		usart2_send(CLIENT_NAK);
	}
}

void pkt_recv(uint8_t* d, size_t dsz) {
	led_on(0);
	//printf("text:'%.*s'\n", (int)dsz, d);
}

void USART2_IRQHandler(void) {
	uint8_t b = (uint8_t)USART2->DR;
	handle(b, pkt_recv);
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
