#include "stm32f4xx.h"

#define LEDS 12

void leds_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~((0xffU << (LEDS * 2))); // ~11'11'11'11
	GPIOD->MODER |= (0x55U << (LEDS * 2)); // 01'01'01'01
}

void blink() {
	const uint32_t led_max_count = 500000;
	static uint32_t led_count = 0;
	led_count = (led_count + 1) % led_max_count;
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

void plli2s_init() {
	// VCOI2S = HSE / PLLM * PLLI2N
	// I2SCLK = VCOI2S / PLLI2R
	// I2SCLK = 8 MHz / 8 * 271 / 2 = 135.5 Mhz
	RCC->PLLI2SCFGR =
		(271U << RCC_PLLI2SCFGR_PLLI2SN_Pos)
		| (2U   << RCC_PLLI2SCFGR_PLLI2SR_Pos);

	RCC->CR |= RCC_CR_PLLI2SON;
	while (!(RCC->CR & RCC_CR_PLLI2SRDY));
}

void spi3clk_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	// PA4 = WS (AF6)
	GPIOA->MODER &= ~(3U << (4 * 2));
	GPIOA->MODER |=  (2U << (4 * 2));
	GPIOA->AFR[0] &= ~(0xFU << (4 * 4));
	GPIOA->AFR[0] |=  (6U << (4 * 4));

	// PC7 = MCK, PC10 = CK, PC12 = SD (AF6)
	GPIOC->MODER &= ~((3U << (7 * 2)) | (3U << (10 * 2)) | (3U << (12 * 2)));
	GPIOC->MODER |=  ((2U << (7 * 2)) | (2U << (10 * 2)) | (2U << (12 * 2)));

	GPIOC->AFR[0] &= ~(0xFU << (7 * 4));
	GPIOC->AFR[0] |=  (6U << (7 * 4));

	GPIOC->AFR[1] &= ~((0xFU << ((10 - 8) * 4)) | (0xFU << ((12 - 8) * 4)));
	GPIOC->AFR[1] |=  ((6U << ((10 - 8) * 4)) | (6U << ((12 - 8) * 4)));

	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

	SPI3->I2SCFGR = 0;

	// F_s = I2SCLK / [256 * (2 * I2SDIV + ODD)]
	// 2 * I2SDIV + ODD = I2SCLK / (256 * F_s)
	// ODD = 0
	// I2SDIV ~= 6
	SPI3->I2SPR =
		SPI_I2SPR_MCKOE
		| (6U << SPI_I2SPR_I2SDIV_Pos);

	SPI3->I2SCFGR = 0;

	// I2SMOD - I2S MODe enable (SPI3 as I2S)
	// I2SCFGR_1 - I2S ConFiGuRation bit 10b
	// I2SCFGR_0 - I2S ConFiGuRation bit 01b
	SPI3->I2SCFGR =
		SPI_I2SCFGR_I2SMOD
		| SPI_I2SCFGR_I2SCFG_1
		| SPI_I2SCFGR_I2SSTD_0;

	SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;
}

// --- I2C interface ---
// DR - Data Register
// SR - Status Register
// SB - Start Bit

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
	// TXE - transmit (TX) Empty
	I2C1->DR = data;
	while (!(I2C1->SR1 & I2C_SR1_TXE));
}

void i2c_read(uint8_t* data) {
	// RXNE - receive (RX) Not Empty
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

void codec_init(void) {
	// Power down codec
	dac_reg_write(0x02, 0x01);
	// Auto-detect MCLK
	dac_reg_write(0x05, 0x81);
	// Interface: I2S, 16-bit
	dac_reg_write(0x06, 0x00);
	// Headphone volume
	dac_reg_write(0x20, (int8_t)0);
	dac_reg_write(0x21, (int8_t)0);
	// Analog power: headphone enabled
	dac_reg_write(0x04, 0xAF);
	// Activate codec
	dac_reg_write(0x02, 0x9E);
}

#include <math.h>

#define TABLE_SIZE 1024

int16_t sine_table[TABLE_SIZE];

void sine_init(void) {
	for (int i = 0; i < TABLE_SIZE; i++) {
		sine_table[i] =
			(int16_t)(sinf(2.0f * 3.1415926f * i / TABLE_SIZE) * 10000.0f);
	}
}

volatile uint32_t phase = 0;

void SPI3_IRQHandler(void)
{
	GPIOD->ODR |= (0x1U << LEDS + 1);
	if (SPI3->SR & SPI_SR_TXE) {
		SPI3->DR = sine_table[phase];
		phase = (phase + 3) & (TABLE_SIZE - 1);
	}
	GPIOD->ODR &= ~(0x1U << LEDS + 1);
}

int main() {
	pll_init();
	leds_init();
	dac_i2c_init();
	codec_init();

	plli2s_init();
	spi3clk_init();

	sine_init();

	NVIC_EnableIRQ(SPI3_IRQn);
	SPI3->CR2 |= SPI_CR2_TXEIE;

	while (1) {
		blink();
	}
}
