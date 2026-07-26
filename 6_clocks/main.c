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
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->CFGR &= ~(0x3 << RCC_CFGR_MCO1_Pos);
	RCC->CFGR |=  (0x0 << RCC_CFGR_MCO1_Pos); // HSI
	// PA8 - Port A, Pin 8
	GPIOA->MODER   &= ~(0x3 << (8 * 2));
	GPIOA->MODER   |=  (0x2 << (8 * 2));
	GPIOA->PUPDR   &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR &= ~(0x3 << (8 * 2));
	GPIOA->OSPEEDR |=  (0x3 << (8 * 2));
	GPIOA->AFR[1]  &= ~(0xF << ((8 - 8) * 4));
}

int clock_index = 0;

void set_clock(uint8_t n) {
	clock_index = n;
	switch (n) {
	case 0:
		RCC->CR |= RCC_CR_HSION;
		while (!(RCC->CR & RCC_CR_HSIRDY));

		RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		RCC->CFGR |=  (0U << RCC_CFGR_MCO1_Pos);

		break;
	case 1:
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR && RCC_CR_HSERDY));

		RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		RCC->CFGR |=  (2U << RCC_CFGR_MCO1_Pos);

		break;
	}
}

void handler() {
	static uint8_t curr = 0;
	curr = (curr + 1) % 2;
	set_clock(curr);
}

void blink() {
	static uint32_t count = 0;
	const uint32_t max_count = 50000;
	count = (count + 1) % max_count;
	GPIOD->ODR &= ~(0xFU << LED1);
	GPIOD->ODR |= ((count > max_count / 2) << LED1 + clock_index);
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
