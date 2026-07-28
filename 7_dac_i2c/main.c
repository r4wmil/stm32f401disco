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
	GPIOD->ODR &= ~(0xfU << LEDS);
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

int main() {
	pll_init();
	leds_init();

	while (1) {
		blink();
	}
}
