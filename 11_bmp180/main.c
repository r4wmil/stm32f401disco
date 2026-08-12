
#include "stm32f4xx.h"
#include <stdio.h>

#define LED 12

void init_leds() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~(0xffU << (LED * 2)); // Clear
	GPIOD->MODER |=  (0x55U << (LED * 2)); // Set 01
}

// --- I2C ---

void init_i2c() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	// PB6, PB9 - AF MODE
	GPIOB->MODER &= ~(3U << (6 * 2));
	GPIOB->MODER |=  (2U << (6 * 2));
	GPIOB->MODER &= ~(3U << (9 * 2));
	GPIOB->MODER |=  (2U << (9 * 2));

	// Setting open-drain mode (driving only low)
	GPIOB->OTYPER |= (1U << 6);
	GPIOB->OTYPER |= (1U << 9);

	GPIOB->OSPEEDR |= (3U << (6 * 2));
	GPIOB->OSPEEDR |= (3U << (9 * 2));

	// So that SCL/SDA would float cus open-drain
	GPIOB->PUPDR &= ~(3U << (6 * 2));
	GPIOB->PUPDR |=  (1U << (6 * 2));
	GPIOB->PUPDR &= ~(3U << (9 * 2));
	GPIOB->PUPDR |=  (1U << (9 * 2));

	GPIOB->AFR[0] &= ~(0xfU << (6 * 4));
	GPIOB->AFR[0] |=  (0x4U << (6 * 4));

	GPIOB->AFR[1] &= ~(0xfU << ((9 - 8) * 4));
	GPIOB->AFR[1] |=  (0x4U << ((9 - 8) * 4));

	// Reset pin high -> 0
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	GPIOD->MODER &= ~(3U << (4 * 2));
	GPIOD->MODER |=  (1U << (4 * 2));
	GPIOD->ODR |= (1U << 4);

	// I2C initialization
	// CRx - Control Register x
	// CCR - Clock Control Register
	// TRISE - Rise Time Register
	I2C1->CR1 = 0;
	// PCLK1 = 16 MHz
	I2C1->CR2 = 16;
	// SCL - Serial Clock Line frequency
	// SCL = 100 kHz (standart mode)
	// CCR = PCLK1 / (2 * SCL)
	I2C1->CCR = 80; // 16 Mhz / (2 * 100 kHz)
	// TRISE - maximal Time of RISE
	// TRISE = 1000 ns / 62.5 ns + 1 ~= 17
	I2C1->TRISE = 17;
	// PE - Peripheral Enable
	I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_wait_byte() {
	while (I2C1->SR2 & I2C_SR2_BUSY);
}

void i2c_start() {
	i2c_wait_byte();
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB));
}

void i2c_write_addr(uint8_t data) {
	I2C1->DR = data;
	while (!(I2C1->SR1 & I2C_SR1_ADDR));
	(void)I2C1->SR2;
}

void i2c_write(uint8_t data) {
	I2C1->DR = data;
	while (!(I2C1->SR1 & I2C_SR1_TXE));
}

void i2c_read(uint8_t* data) {
	while (!(I2C1->SR1 & I2C_SR1_RXNE));
	*data = I2C1->DR;
}

void i2c_stop() {
	I2C1->CR1 |= I2C_CR1_STOP;
}

#define I2C_ADDR 0xEE

void i2c_reg_write(uint8_t reg, uint8_t val) {
	i2c_start();
	i2c_write_addr(I2C_ADDR & ~1U);
	i2c_write(reg);
	i2c_write(val);
	i2c_stop();
}

uint8_t i2c_reg_read(uint8_t reg) {
	uint8_t data;

	i2c_wait_byte();

	i2c_start();
	i2c_write_addr(I2C_ADDR & ~1U);
	i2c_write(reg);
	i2c_stop();

	i2c_start();
	i2c_write_addr(I2C_ADDR | 1U);

	I2C1->CR1 &= ~I2C_CR1_ACK;

	i2c_stop();

	i2c_read(&data);

	I2C1->CR1 |= I2C_CR1_ACK;

	return data;
}

// --- USART2 ---

void init_usart2() {
	// 9600 - baud rate
	// 8 - data bits
	// 1 - stop bit
	// None - parity
	// None - flow control
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

void usart2_send(uint8_t b) {
	while (!(USART2->SR & USART_SR_TXE));
	USART2->DR = b;
}

int _write(int file, char *ptr, int len) {
	for (int i = 0; i < len; i++) {
		usart2_send(ptr[i]);
	}
	return len;
}

// --- MAIN ---

uint8_t buf[22];

int main(void) {

	init_leds();
	init_usart2();
	init_i2c();

	printf("\033[2J\033[H");
	printf("hello\r\n");
	while (1) {
		printf("%x\r\n", i2c_reg_read(0xD0));
		//for (int i = 0; i < 22; i++) printf("%x\r\n", buf[i]);
		GPIOD->ODR ^= (1U << LED + 0);
		for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
