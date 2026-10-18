#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define HELPER_IMPLEMENTATION
#include "helper.h"
#undef  HELPER_IMPLEMENTATION

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
	AM_ECHO,
	AM_TTT
} app_mode_t;

app_mode_t app_mode = 0;

bool check_command(uint8_t* com, size_t len, uint8_t* buf) {
	size_t com_len = strlen(com);
	if (com_len != len) return false;
	if (strncmp(com, buf, len)) return false;
	return true;
}

void com_help() {
	printf("commands help:\r\n");
	printf("'echo' - echo mode\r\n");
	printf("'ttt' - tictactoe game (TODO)\r\n");
	printf("'r' - system reset\r\n");
}

void com_hello() {
	printf("\r\n\r\nhello!\r\n");
	com_help();
}

int com_modes(size_t* top, uint8_t* buf) {
	switch (app_mode) {
		case AM_ECHO:
			if (check_command("q", *top, buf)) { app_mode = AM_COMMAND; return 2; }
			printf("\r\n%.*s\r\n", (int)*top, buf);
			*top = 0;
			return 1;
			break;
		case AM_TTT:
			if (check_command("q", *top, buf)) { app_mode = AM_COMMAND; return 2; }
			printf("\r\nTICTACTOE: %.*s\r\n", (int)*top, buf);
			*top = 0;
			return 1;
			break;
	}
	return 0;
}

void com_commands() {
#define BUF_MAX 64
	static uint8_t buf[BUF_MAX];
	static size_t top = 0;
	uint8_t b = usart2_recv();
	bool is_reset = false;
	if (b) {
		if (b == '\r') {
			int code = com_modes(&top, buf);
			if (code == 1) { return; }
			if (code == 2) {
				com_hello();
				return;
			}
			//for (size_t i = 0; i < top; i++) usart2_send(buf[i]);
			printf("\r\n");
			if (check_command("echo", top, buf)) {
				app_mode = AM_ECHO;
				printf("echo mode enabled. type \"q\\n\" to quit\r\n");
			} else if (check_command("r", top, buf)) {
				printf("reseting...\r\n");
				is_reset = true;
			} else if (check_command("ttt", top, buf)) {
				app_mode = AM_TTT;
				printf("tictactoe mode enabled. type \"q\\n\" to quit\r\n");
			} else { com_help(); }
			top = 0;
		} else {
			if (b == '\b' || b == 0x7F) {
				buf[top] = '\0';
				if (top != 0) top--;
			} else {
				buf[top] = b;
				if (top < BUF_MAX) { top++; }
			}
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
	com_hello();
	xTaskCreate(task_com, "task_com", 2048, NULL, 1, NULL);
	xTaskCreate(task_blink1, "task_blink1", 128, NULL, 1, NULL);
	xTaskCreate(task_blink2, "task_blink2", 128, NULL, 1, NULL);
	vTaskStartScheduler();
	while (1);
}
