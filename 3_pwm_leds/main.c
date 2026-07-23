#include "stm32f4xx.h"

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15

int main(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	// APB - Advanced Peripheral Bus
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	// Port D: MODE Register
	GPIOD->MODER &= ~(3U << (LED1 * 2)); // Clear
	GPIOD->MODER |= (2U << (LED1 * 2)); // Alternate function
	// AFR - Alternate Function Register
	// Alternate Function 2 - AFR[1]
	GPIOD->AFR[1] |= (2U << ((LED1 - 8) * 4)); // 010 - TIM3-5

	// PreSCaler
	TIM4->PSC = 84 - 1; // 84 MHz / 84 = 1 MHz
	// Auto-Reload Register
	TIM4->ARR = 1000 - 1; // 1 MHz / 1000 = 1 kHz
	// CCR1 - Capture/Compare Register 1
	TIM4->CCR1 = 10; // 500 / 1000 = 50% duty
	// CCMR1 - Capture/Compare Mode Register 1
	TIM4->CCMR1 = (6U << 4); // 110 - PWM Mode 1 (<duty -> high; >duty -> low)
	// CCER - Capture/Compare Enable Register
	TIM4->CCER |= (1U << 0);
	// CR1 - Control Register 1
	TIM4->CR1 |= (1U << 0);
	
	while (1) {
		__NOP();
	}
}
