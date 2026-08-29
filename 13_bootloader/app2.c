#include "stm32f4xx.h"

#define LEDS 12

void leds_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~((0xffU << (LEDS * 2))); // ~11'11'11'11
	GPIOD->MODER |= (0x55U << (LEDS * 2)); // 01'01'01'01
}

int main() {
	leds_init();
	while (1) {
		GPIOD->ODR ^= (0x1U << LEDS + 2);
		for (volatile int i = 0; i < 500000; i++);
	}
	return 0;
}


