// 9600 - baud rate
// 8 - data bits
// 1 - stop bit
// None - parity
// None - flow control

#include "stm32f4xx.h"

#define LED 12

void init_leds() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~(0xffU << (LED * 2)); // Clear
	GPIOD->MODER |=  (0x55U << (LED * 2)); // Set 01
}

void init_usart2() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	
	GPIOD->MODER &= ~((3U << (5 * 2)) | (3U << (6 * 2)));
	GPIOD->MODER |=  ((2U << (5 * 2)) | (2U << (6 * 2)));

	GPIOD->AFR[0] &= ~(0xFU << (5 * 4) | (0xFU << (6 * 4)));
	GPIOD->AFR[0] |=  (0x7U << (5 * 4) | (0x7U << (6 * 4)));

	// BRR - Baud Rate Register
	// TE - Transmitter Enable
	// UE - USART Enable
	// 16 MHz / 9600 baud rate
	USART2->BRR = 16000000 / 9600;
	USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

int main(void) {

	init_leds();
	init_usart2();

	uint8_t c = 0;
	while (1) {
		while (!(USART2->SR & USART_SR_TXE));
		USART2->DR = c++;
		GPIOD->ODR ^= (1U << LED + 0);
		for (volatile uint32_t i = 0; i < 50000; i++);
	}
}
