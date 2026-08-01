#include "stm32f4xx.h"

#define LEDS 12

void leds_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~((0xffU << (LEDS * 2))); // ~11'11'11'11
	GPIOD->MODER |= (0x55U << (LEDS * 2)); // 01'01'01'01
}

void blink() {
	const uint32_t led_max_count = 500000;
	uint32_t led_count = (led_count + 1) % led_max_count;
	GPIOD->ODR &= ~(0x1U << LEDS);
	GPIOD->ODR |= ((led_count < led_max_count / 2) << LEDS);
}

void pll_init() {
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY));

	FLASH->ACR = FLASH_ACR_LATENCY_2WS;

	RCC->CFGR =
		  RCC_CFGR_HPRE_DIV1
		| RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_PPRE2_DIV1;

	RCC->PLLCFGR =
		  (8U   << RCC_PLLCFGR_PLLM_Pos)
		| (336U << RCC_PLLCFGR_PLLN_Pos)
		| (1U   << RCC_PLLCFGR_PLLP_Pos)
		| (7U   << RCC_PLLCFGR_PLLQ_Pos)
		| RCC_PLLCFGR_PLLSRC_HSE;

	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));

	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

	SystemCoreClockUpdate();
}

// I2C - Inter-Integrated Circuit

#define I2C_ADDR 0x94

void dac_i2c_init() {
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
	// PCLK1 = 42 MHz
	I2C1->CR2 = 42;
	// SCL - Serial Clock Line frequency
	// SCL = 100 MHz (standart mode)
	// CCR = PCLK1 / (2 * SCL)
	I2C1->CCR = 210;
	// TRISE - maximal Time of RISE
	// TRISE = (42 MHz * 1000 ns) + 1 = 43
	// - for under 1000 ns rise time
	I2C1->TRISE = 43;
	// PE - Peripheral Enable
	I2C1->CR1 |= I2C_CR1_PE;
}

// --- I2C interface ---

// DR - Data Register
// SR - Status Register
// SB - Start Bit

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
	// TXE - transmit (TX) buffer Empty
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

void dac_reg_write(uint8_t reg, uint8_t val) {
	i2c_start();
	i2c_write_addr(I2C_ADDR & ~1U);
	i2c_write(reg);
	i2c_write(val);
	i2c_stop();
}

uint8_t dac_reg_read(uint8_t reg) {
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

int main() {
	pll_init();
	leds_init();
	dac_i2c_init();

	uint8_t cid = dac_reg_read(0x01) & 0xf8;
	GPIOD->ODR |= ((cid == 0xe0) << LEDS + 1);

	dac_reg_write(0x02, 0x9E);
	uint8_t v = dac_reg_read(0x02);
	GPIOD->ODR |= ((v == 0x9E) << LEDS + 2);

	while (1) {
		blink();
	}
}
