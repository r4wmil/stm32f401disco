#include "stm32f4xx.h"
#include <stdbool.h>

#define BTN 0
#define LED1 12

bool read_btn() {
	// IDR - Input Data Register
	return (GPIOA->IDR & (1U << BTN)) != 0;
}

void btn_event() {
	static bool btn_pressed = false;
	static int debounce = -1;
	if (read_btn()) {
		if (btn_pressed) return;
		if (debounce == -1) debounce = 1000;
		if (debounce > 0) { debounce--; return; }
		if (read_btn()) {
			GPIOD->ODR ^= (1U << LED1);
			btn_pressed = true;
		}
		return;
	}
	debounce = -1;
	btn_pressed = false;
}

int main(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	GPIOD->MODER &= ~(3U << (LED1 * 2));
	GPIOD->MODER |=  (1U << (LED1 * 2));

	GPIOA->MODER &= ~(3U << (BTN  * 2)); // >00 - input
	// PUPDR - Pull-Up / Pull-Down Register
	GPIOA->PUPDR &= ~(3U << (BTN  * 2)); // >00 - no pull-up/pull-down

	while (1) {
		btn_event();
	}
}
