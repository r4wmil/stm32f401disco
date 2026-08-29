#include "stm32f4xx.h"

#define APP1_ADDR 0x08008000
#define APP2_ADDR 0x08020000

void jump_to_app(uint32_t addr) {
	uint32_t sp = *(uint32_t *)addr;
	uint32_t reset = *(uint32_t *)(addr + 4);

	__disable_irq();
	SCB->VTOR = addr;
	__set_MSP(sp);

	((void (*)(void))reset)();
}

int main(void) {
	jump_to_app(APP1_ADDR);

	while (1);
}
