#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define LEDS 12

// --- SYSTEM ---

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

// --- USART ---

void usart2_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	GPIOD->MODER &= ~((3U << (5 * 2)) | (3U << (6 * 2)));
	GPIOD->MODER |=  ((2U << (5 * 2)) | (2U << (6 * 2)));

	GPIOD->AFR[0] &= ~(0xFU << (5 * 4) | (0xFU << (6 * 4)));
	GPIOD->AFR[0] |=  (0x7U << (5 * 4) | (0x7U << (6 * 4)));

	// BRR - Baud Rate Register
	// TE - Transmitter Enable
	// UE - USART Enable
	// 42 MHz (APB1 spped) / 9600 baud rate
	USART2->BRR = 42000000 / 9600;
	USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void usart2_send(uint8_t b) {
	while (!(USART2->SR & USART_SR_TXE));
	USART2->DR = b;
}

uint8_t usart2_recv() {
	return !(USART2->SR & USART_SR_RXNE) ? 0x0 : USART2->DR;
}

int _write(int file, char *ptr, int len) {
	for (int i = 0; i < len; i++) {
		usart2_send(ptr[i]);
	}
	return len;
}

// --- TASKS ---

void task_blink1(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 0));
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void task_blink2(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 1));
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

#define BUF_SIZE 256

// --- COMMANDS ---

typedef enum {
	AM_COMMAND = 0,
	AM_ECHO
} app_mode_t;

app_mode_t app_mode = 0;

bool check_command(uint8_t* com, size_t len, uint8_t* buf) {
	size_t com_len = strlen(com);
	if (com_len != len) return false;
	if (strncmp(com, buf, len)) return false;
	return true;
}

void com_commands() {
#define BUF_MAX 64
	static uint8_t buf[BUF_MAX];
	static size_t top = 0;
	uint8_t b = usart2_recv();
	bool is_reset = false;
	if (b) {
		if (b == '\r') {
			if (app_mode == AM_ECHO) {
				if (check_command("q", top, buf)) { app_mode = AM_COMMAND; return; }
				printf("\r\n%.*s\r\n", (int)top, buf);
				top = 0;
				return;
			}
			//for (size_t i = 0; i < top; i++) usart2_send(buf[i]);
			printf("\r\n");
			if (check_command("echo", top, buf)) {
				app_mode = AM_ECHO;
				printf("echo mode enabled. type \"q\\n\" to quit\r\n");
			} else if (check_command("reset", top, buf)) {
				printf("reseting...\r\n");
				is_reset = true;
			} else {
				printf("commands help:\r\n");
				printf("'echo' - echo mode\r\n");
				printf("'reset' - system reset\r\n");
			}
			top = 0;
		} else {
			buf[top] = b;
			if (top < BUF_MAX) { top++; }
			usart2_send(b);
		}
	}
	GPIOD->ODR ^= (1U << LEDS + 2);
	vTaskDelay(pdMS_TO_TICKS(50));
	if (is_reset) {
		NVIC_SystemReset();
	}
}

void task_com(void *arg) {
	setvbuf(stdout, NULL, _IONBF, 0);
	while (1) {
		com_commands();
	}
}

// --- MAIN ---

int main(void) {
	pll_init();
	mco_init();
	leds_init();
	usart2_init();
	// xTaskCreate(fn, name, stack_depth, fn_params, priority, task_ptr);
	xTaskCreate(task_com, "task_com", 2048, NULL, 1, NULL);
	xTaskCreate(task_blink1, "task_blink1", 128, NULL, 1, NULL);
	xTaskCreate(task_blink2, "task_blink2", 128, NULL, 1, NULL);
	vTaskStartScheduler();
	while (1);
}
