#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

enum {
	STATE_PENDING = 0,
	STATE_FLASHING
};

int state = STATE_PENDING;

void state_pending(uint8_t b) {
	static bool sent = 0;
	if (!sent && b == 0xF1) {
		usart2_send(0x1E);
		GPIOD->ODR &= ~(1U << LEDS + 2);
		sent = 1;
		state = STATE_FLASHING;
	}
}

void state_flashing(uint8_t b) {
	if (b) {
		usart2_send(b);
		if (b == 0x12)
			GPIOD->ODR |=  (1U << LEDS + 1);
	}
}

void handle(uint8_t b) {
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_FLASHING:
		state_flashing(b);
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
