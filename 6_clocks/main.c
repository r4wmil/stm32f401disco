#include "stm32f4xx.h"

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15

int main(void) {
	// --- LEDs setup ---
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~(
		  (0x3 << (LED1 * 2))
		& (0x3 << (LED2 * 2))
		& (0x3 << (LED3 * 2))
		& (0x3 << (LED4 * 2)));
	GPIOD->MODER |=
		  (0x1 << (LED1 * 2))
		| (0x1 << (LED2 * 2))
		| (0x1 << (LED3 * 2))
		| (0x1 << (LED4 * 2));

	// --- MCO1 setup ---
	// MCO1 - Microcontroller Clock Output 1
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->CFGR &= ~(0x3 << RCC_CFGR_MCO1_Pos);
	RCC->CFGR |=  (0x0 << RCC_CFGR_MCO1_Pos); // HSI

	// PA8 - Port A, Pin 8
	GPIOA->MODER   &= ~(0x3 << (8 * 2));
	GPIOA->MODER   |=  (0x2 << (8 * 2));
	GPIOA->PUPDR   &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR |=  (0x3 << (8 * 2));
	GPIOA->AFR[1]  &= ~(0xF << ((8 - 8) * 4));

	while (1) {
		GPIOD->ODR ^= (1U << LED1);
		for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
