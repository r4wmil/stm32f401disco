
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

int32_t bmp180_pressure(void) {
	// Calibration coefficients
	int16_t AC1 = (i2c_reg_read(0xAA) << 8) | i2c_reg_read(0xAB);
	int16_t AC2 = (i2c_reg_read(0xAC) << 8) | i2c_reg_read(0xAD);
	int16_t AC3 = (i2c_reg_read(0xAE) << 8) | i2c_reg_read(0xAF);
	uint16_t AC4 = (i2c_reg_read(0xB0) << 8) | i2c_reg_read(0xB1);
	uint16_t AC5 = (i2c_reg_read(0xB2) << 8) | i2c_reg_read(0xB3);
	uint16_t AC6 = (i2c_reg_read(0xB4) << 8) | i2c_reg_read(0xB5);
	int16_t B1  = (i2c_reg_read(0xB6) << 8) | i2c_reg_read(0xB7);
	int16_t B2  = (i2c_reg_read(0xB8) << 8) | i2c_reg_read(0xB9);
	int16_t MB  = (i2c_reg_read(0xBA) << 8) | i2c_reg_read(0xBB);
	int16_t MC  = (i2c_reg_read(0xBC) << 8) | i2c_reg_read(0xBD);
	int16_t MD  = (i2c_reg_read(0xBE) << 8) | i2c_reg_read(0xBF);

	// Temperature
	i2c_reg_write(0xF4, 0x2E);
	for (volatile int i = 0; i < 100000; i++);
	int32_t UT = (i2c_reg_read(0xF6) << 8) | i2c_reg_read(0xF7);

	int32_t X1 = ((UT - AC6) * AC5) >> 15;
	int32_t X2 = (MC << 11) / (X1 + MD);
	int32_t B5 = X1 + X2;

	// Pressure, OSS = 0
	i2c_reg_write(0xF4, 0x34);
	for (volatile int i = 0; i < 100000; i++);
	int32_t UP = (i2c_reg_read(0xF6) << 8) | i2c_reg_read(0xF7);

	int32_t B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6 >> 12)) >> 11;
	X2 = (AC2 * B6) >> 11;
	int32_t X3 = X1 + X2;
	int32_t B3 = (((AC1 * 4 + X3) + 2) / 4);

	X1 = (AC3 * B6) >> 13;
	X2 = (B1 * (B6 * B6 >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;

	uint32_t B4 = (AC4 * (uint32_t)(X3 + 32768)) >> 15;
	uint32_t B7 = ((uint32_t)UP - B3) * 50000;

	int32_t p = (B7 < 0x80000000) ?
		(B7 * 2) / B4 : (B7 / B4) * 2;

	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;

	return p + ((X1 + X2 + 3791) >> 4);
}

int main(void) {

	init_leds();
	init_usart2();
	init_i2c();

	printf("\033[2J\033[H");
	printf("%x\r\n", i2c_reg_read(0xD0));
	while (1) {
		printf("pressure: %ld pa\r\n", bmp180_pressure());
		//for (int i = 0; i < 22; i++) printf("%x\r\n", buf[i]);
		GPIOD->ODR ^= (1U << LED + 0);
		for (volatile uint32_t i = 0; i < 800000; i++);
	}
}
