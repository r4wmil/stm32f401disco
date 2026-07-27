// This example blinking the lights with code so you can
// feel the the SYSCLK speed 
// Probing PA8 shows system clock signal

#include "stm32f4xx.h"

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15
#define BTN  0

void btn_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= ~(0x3 << (BTN * 2));
	GPIOA->PUPDR &= ~(0x3 << (BTN * 2));
}

bool btn_read() {
	return (GPIOA->IDR & (1U << BTN)) != 0;
}

typedef void (*fn_type)();

void btn_event(fn_type fn) {
	static bool btn_pressed = false;
	static int debounce = -1;
	if (btn_read()) {
		if (btn_pressed) return;
		if (debounce == -1) debounce = 1000;
		if (debounce > 0) { debounce--; return; }
		if (btn_read()) {
			fn();
			btn_pressed = true;
		}
		return;
	}
	debounce = -1;
	btn_pressed = false;
}

void leds_init() {
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
}

void mco_init() {
	// MCO1 - Microcontroller Clock Output 1
	// Port A, Pin 8 (PA8)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->CFGR &= ~(0x3 << RCC_CFGR_MCO1_Pos);
	RCC->CFGR |=  (0x0 << RCC_CFGR_MCO1_Pos); // HSI
	GPIOA->MODER   &= ~(0x3 << (8 * 2));
	GPIOA->MODER   |=  (0x2 << (8 * 2));
	GPIOA->PUPDR   &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR |=  (0x3 << (8 * 2));
	GPIOA->AFR[1]  &= ~(0xF << ((8 - 8) * 4));
	// OSPEEDR - Output SPEED Register
}

int clock_index = 0;
uint32_t led_count = 0;

void set_clock(uint8_t n) {
	clock_index = n;
	switch (n) {
	case 0:
		// RCC - Reset & Clock Control
		// CR - Control Register
		// HSERDY - High-Speed External ReaDY
		RCC->CR |= RCC_CR_HSION;
		while (!(RCC->CR & RCC_CR_HSIRDY));

		// CFGR - (Clock) ConFiGuration Register
		// SW - (System Clock) SWitch
		// SWS - (System Clock) SWich Status
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_HSI;
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

		// MCO - Microcontroller Clock Output 1
		RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		RCC->CFGR |=  (0U << RCC_CFGR_MCO1_Pos);

		// Disable PLL & HSE
		RCC->CR &= ~RCC_CR_PLLON;
		while (RCC->CR & RCC_CR_PLLRDY);
		RCC->CR &= ~RCC_CR_HSEON;
		while (RCC->CR & RCC_CR_HSERDY);

		break;
	case 1:
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY));

		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_HSE;
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);

		RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		RCC->CFGR |=  (2U << RCC_CFGR_MCO1_Pos);

		// Disable HSI
		RCC->CR &= ~RCC_CR_HSION;
		while (RCC->CR & RCC_CR_HSIRDY);

		break;
	case 2:
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY));

		// Ensuring flash memory will properly work
		// ACR - Access Control Register
		FLASH->ACR = FLASH_ACR_LATENCY_2WS;

		// Optional performance optimizations:
		// - FLASH_ACR_ICEN - Instruction Cache
		// - FLASH_ACR_DCEN - Data Cache
		// - FLASH_ACR_PRFTEN - PReFetch

		// AHB - Advanced High-performance Bus
		// - CPU, SRAM, DMA, GPIO, etc.
		// APB - Advanced Peripheral Bus
		// - UART, SPI, I2C, ADC, etc.
		// AHB = 84 MHz, APB1 = 42 MHz, APB2 = 84 MHz
		RCC->CFGR =
			  RCC_CFGR_HPRE_DIV1
			| RCC_CFGR_PPRE1_DIV2
			| RCC_CFGR_PPRE2_DIV1;

		// VCO - Voltage-Controlled Oscilator
		// VCO = (Clock Source / PLLM) * PLLN
		// - Clock Source is HSE
		// SYSCLK = VCO / PLLP
		// PLL48CLK = VCO / PLLQ
		// SYSCLK   = 8 MHz / 8 * 336 / 4 = 84 MHz
		// PLL48CLK = 8 MHz / 8 * 336 / 7 = 48 MHz
		// PLLQ - for peripherals like USB OTG FS, SDIO, RNG
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

		SystemCoreClockUpdate(); // Sets SystemCoreClock value

		RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		RCC->CFGR |=  (3U << RCC_CFGR_MCO1_Pos);

		break;
	}
	led_count = 0;
}

void handler() {
	static uint8_t curr = 0;
	curr = (curr + 1) % 3;
	set_clock(curr);
}

void blink() {
	const uint32_t led_max_count = 50000;
	led_count = (led_count + 1) % led_max_count;
	GPIOD->ODR &= ~(0xFU << LED1);
	GPIOD->ODR |= ((led_count < led_max_count / 2) << LED1 + clock_index);
}

int main(void) {
	btn_init();
	leds_init();
	mco_init();

	set_clock(0);

	while (1) {
		blink();
		btn_event(handler);
	}
}
