#include "stm32f4xx.h"

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15

void init_pin(unsigned int PIN) {
	GPIOD->MODER &= ~(3U << (PIN * 2));
	GPIOD->MODER |=  (1U << (PIN * 2));
}

int main(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	init_pin(LED1);
	init_pin(LED2);
	init_pin(LED3);
	init_pin(LED4);

	while (1)
	{
		GPIOD->ODR ^= (1U << LED1);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED2);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED3);
		for (volatile uint32_t i = 0; i < 500000; i++);
		GPIOD->ODR ^= (1U << LED4);
		for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
