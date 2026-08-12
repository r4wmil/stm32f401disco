#include "stm32f4xx.h"

#define LED 12

void init_leds() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~(0xffU << (LED * 2)); // Clear
	GPIOD->MODER |=  (0x55U << (LED * 2)); // Set 01
}

int main(void) {

	init_leds();

	while (1) {
		GPIOD->ODR ^= (1U << LED + 0);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED + 1);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED + 2);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED + 3);
		for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
