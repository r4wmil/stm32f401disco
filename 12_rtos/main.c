#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

#define LEDS 12

void leds_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~((0xffU << (LEDS * 2))); // ~11'11'11'11
	GPIOD->MODER |= (0x55U << (LEDS * 2)); // 01'01'01'01
}

void task1(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 0));
		for (volatile int i = 0; i < 100000; i++);
	}
}

void task2(void *arg) {
	while (1) {
		GPIOD->ODR ^= (0x1U << (LEDS + 1));
		for (volatile int i = 0; i < 200000; i++);
	}
}

int main(void) {
	leds_init();
	xTaskCreate(task1, "task1", 128, NULL, 1, NULL);
	xTaskCreate(task2, "task2", 128, NULL, 1, NULL);
	vTaskStartScheduler();
	while (1);
}
