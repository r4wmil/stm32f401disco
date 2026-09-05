#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#define LEDS 12

void leds_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~((0xffU << (LEDS * 2))); // ~11'11'11'11
	GPIOD->MODER |= (0x55U << (LEDS * 2)); // 01'01'01'01
}

void pll_init() {
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

	// Enable for PLL
	RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
	RCC->CFGR |= (3U << RCC_CFGR_MCO1_Pos);  // PLL
}

void task1(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 0));
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void task2(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 1));
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

int main(void) {
	pll_init();
	mco_init();
	leds_init();
	// xTaskCreate(fn, name, stack_depth, fn_params, priority, task_ptr);
	xTaskCreate(task1, "task1", 128, NULL, 1, NULL);
	xTaskCreate(task2, "task2", 128, NULL, 0, NULL);
	vTaskStartScheduler();
	while (1);
}
