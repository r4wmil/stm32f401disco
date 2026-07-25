#include "stm32f4xx.h"
#include "math.h"

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15

int main(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	GPIOD->MODER &=
		  ~(3U << (LED1 * 2))
		& ~(3U << (LED2 * 2))
		& ~(3U << (LED3 * 2))
		& ~(3U << (LED4 * 2));
	GPIOD->MODER |=
		  (2U << (LED1 * 2))
		| (2U << (LED2 * 2))
		| (2U << (LED3 * 2))
		| (2U << (LED4 * 2));

	// AFR - Alternate Function Register
	// AFR[0] - pins 1-7
	// AFR[1] - pins 8-15
	GPIOD->AFR[1] =
		  (2U << ((LED1 - 8) * 4))
		| (2U << ((LED2 - 8) * 4))
		| (2U << ((LED3 - 8) * 4))
		| (2U << ((LED4 - 8) * 4));

	TIM4->PSC = 84 - 1; // PreSCaler
	TIM4->ARR = 1000 - 1; // Auto-Reload Register

	TIM4->CCMR1 = // Capture/Compare Mode Register 1
		  (6U << 4)   // Channel 1: Mode =110
		| (1U << 3)   //            Preload Enable
		| (6U << 12)  // Channel 2: Mode =110
		| (1U << 11); //            Preload Enable

	TIM4->CCMR2 = // Capture/Compare Mode Register 2
		  (6U << 4)   // Channel 3: Mode =110
		| (1U << 3)   //            Preload Enable
		| (6U << 12)  // Channel 4: Mode =110
		| (1U << 11); //            Preload Enable

	TIM4->CCER = // Capture/Compare Enable Register
		  TIM_CCER_CC1E  // Channel 1 Enable
		| TIM_CCER_CC2E  // Channel 2 Enable
		| TIM_CCER_CC3E  // Channel 3 Enable
		| TIM_CCER_CC4E; // Channel 4 Enable

	// CR1 - Control Register 1
	TIM4->CR1 |= TIM_CR1_ARPE; // Auto-Reload Preload Enable

	// EGR - Event Generation Register
	TIM4->EGR = TIM_EGR_UG; // Update Generation (?)
				//
	TIM4->CR1 |= TIM_CR1_CEN;  // Counter ENable

	float t = 0.0f, t1, t2, t3, t4;
	while (1) {
		t1 = sinf(2*M_PI*t + M_PI*0.00f)*0.5f + 0.5f;
		t2 = sinf(2*M_PI*t + M_PI*0.25f)*0.5f + 0.5f;
		t3 = sinf(2*M_PI*t + M_PI*0.50f)*0.5f + 0.5f;
		t4 = sinf(2*M_PI*t + M_PI*0.75f)*0.5f + 0.5f;
		// CCR - Compare/Capture Registers
		TIM4->CCR1 = 1000 * powf(t1, 2.0f);
		TIM4->CCR2 = 1000 * powf(t2, 2.0f);;
		TIM4->CCR3 = 1000 * powf(t3, 2.0f);;
		TIM4->CCR4 = 1000 * powf(t4, 2.0f);;
		t += 0.0005f;
		if (t > 1.0f) t = 0.0f;
	}
}
