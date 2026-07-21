#include "stm32f4xx.h"

#define CONV_IMPLEMENTATION
#include "conv.h"
#undef CONV_IMPLEMENTATION

#define LED1 12
#define LED2 13
#define LED3 14
#define LED4 15

void init_pin(uint32_t PIN) {
	GPIOD->MODER &= ~(3U << (PIN * 2));
	GPIOD->MODER |=  (1U << (PIN * 2));
}

#define UT_ASSERT(_cond) if (!(_cond)) { return false; }

bool ut_enc(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	bool* inp, size_t ilen,
	bool* exp, size_t elen) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	UT_ASSERT(t.constr_len == constr_len);
	UT_ASSERT(t.input_len == input_len);
	UT_ASSERT(t.input_syms == input_syms);
	UT_ASSERT(t.output_syms == output_syms);
	UT_ASSERT(t.states_num == states_num);
	UT_ASSERT(t.next_states);
	UT_ASSERT(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(outputs[i] == t.outputs[i]); }
	size_t olen = conv_get_enc_len(t, ilen);
	UT_ASSERT(olen == elen);
	bool* out = calloc(olen, sizeof(*out));
	conv_enc(t, inp, ilen, out);
	for (size_t i = 0; i < olen; i++) { UT_ASSERT(exp[i] == out[i]); }
}

bool ut_hard(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	bool* inp, size_t ilen,
	bool* exp) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	UT_ASSERT(t.constr_len == constr_len);
	UT_ASSERT(t.input_len == input_len);
	UT_ASSERT(t.input_syms == input_syms);
	UT_ASSERT(t.output_syms == output_syms);
	UT_ASSERT(t.states_num == states_num);
	UT_ASSERT(t.next_states);
	UT_ASSERT(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(outputs[i] == t.outputs[i]); }
	size_t blen = conv_get_enc_len(t, ilen);
	bool* buf = calloc(blen, sizeof(*buf));
	bool* out = calloc(ilen, sizeof(*out));
	conv_enc(t, inp, ilen, buf);
	conv_hard(t, buf, blen, out);
	for (size_t i = 0; i < ilen; i++) { UT_ASSERT(exp[i] == out[i]); }
}

bool ut_soft(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	float* inp, size_t ilen,
	bool* exp, size_t elen) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	UT_ASSERT(t.constr_len == constr_len);
	UT_ASSERT(t.input_len == input_len);
	UT_ASSERT(t.input_syms == input_syms);
	UT_ASSERT(t.output_syms == output_syms);
	UT_ASSERT(t.states_num == states_num);
	UT_ASSERT(t.next_states);
	UT_ASSERT(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { UT_ASSERT(outputs[i] == t.outputs[i]); }
	bool* out = calloc(elen, sizeof(*out));
	conv_soft(t, inp, ilen, out);
}

int main(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	init_pin(LED1);
	init_pin(LED2);
	init_pin(LED3);
	init_pin(LED4);

	GPIOD->ODR |= (1U << LED4);

	bool enc_res, hard_res, soft_res;

	enc_res = ut_enc(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(bool[]){1,1,0,1}, 4,
		(bool[]){1,1, 1,0, 1,0, 0,0}, 8);
	hard_res = ut_hard(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(bool[]){1,1,0,1}, 4,
		(bool[]){1,1,0,1});
	soft_res = ut_soft(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(float[]){-0.8,-0.6, +0.1,+1.3, -2.0,+0.9, +1.0,+0.3}, 8,
		(bool[]){1,1,0,1}, 4);

	GPIOD->ODR &= ~(1U << LED4);

	if (enc_res) GPIOD->ODR |= (1U << LED1);
	if (hard_res) GPIOD->ODR |= (1U << LED2);
	if (soft_res) GPIOD->ODR |= (1U << LED3);

	while (1) {
		for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
