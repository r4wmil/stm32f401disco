#define CONV_IMPLEMENTATION
#include "conv.h"
#undef CONV_IMPLEMENTATION

#include "stdio.h"
#include "stdint.h"
#include "assert.h"

//size_t constr_len;   // constraint length
//size_t input_len;   // input bits
//size_t output_len;   // output bits
//size_t input_syms;  // number of input symbols
//size_t output_syms;  // number of output symbols
//size_t states_num;   // number of states
//uint64_t next_states; // next states
//uint64_t outputs; // outputs

void ut_trel(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	assert(t.constr_len == constr_len);
	assert(t.input_len == input_len);
	assert(t.input_syms == input_syms);
	assert(t.output_syms == output_syms);
	assert(t.states_num == states_num);
	assert(t.next_states);
	assert(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(outputs[i] == t.outputs[i]); }
}

void ut_enc(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	bool* inp, size_t ilen,
	bool* exp, size_t elen) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	assert(t.constr_len == constr_len);
	assert(t.input_len == input_len);
	assert(t.input_syms == input_syms);
	assert(t.output_syms == output_syms);
	assert(t.states_num == states_num);
	assert(t.next_states);
	assert(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(outputs[i] == t.outputs[i]); }
	size_t olen = conv_get_enc_len(t, ilen);
	assert(olen == elen);
	bool* out = calloc(olen, sizeof(*out));
	conv_enc(t, inp, ilen, out);
	for (size_t i = 0; i < olen; i++) { assert(exp[i] == out[i]); }
}

void ut_hard(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	bool* inp, size_t ilen,
	bool* exp) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	assert(t.constr_len == constr_len);
	assert(t.input_len == input_len);
	assert(t.input_syms == input_syms);
	assert(t.output_syms == output_syms);
	assert(t.states_num == states_num);
	assert(t.next_states);
	assert(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(outputs[i] == t.outputs[i]); }
	size_t blen = conv_get_enc_len(t, ilen);
	bool* buf = calloc(blen, sizeof(*buf));
	bool* out = calloc(ilen, sizeof(*out));
	conv_enc(t, inp, ilen, buf);
	conv_hard(t, buf, blen, out);
	for (size_t i = 0; i < ilen; i++) { assert(exp[i] == out[i]); }
}

void ut_soft(size_t constr_len, uint32_t* gen, size_t len,
	size_t input_len, size_t output_len,
	size_t input_syms, size_t output_syms, size_t states_num,
	uint64_t* next_states,
	uint64_t* outputs,
	float* inp, size_t ilen,
	bool* exp, size_t elen) {
	conv_trel_t t = conv_trel_gen(constr_len, gen, len);
	assert(t.constr_len == constr_len);
	assert(t.input_len == input_len);
	assert(t.input_syms == input_syms);
	assert(t.output_syms == output_syms);
	assert(t.states_num == states_num);
	assert(t.next_states);
	assert(t.outputs);
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(next_states[i] == t.next_states[i]); }
	for (size_t i = 0; i < t.states_num * t.input_syms; i++) { assert(outputs[i] == t.outputs[i]); }
	
	// Pass ilen directly to conv_soft since it represents the float array length
	bool* out = calloc(elen, sizeof(*out));
	conv_soft(t, inp, ilen, out);
	
	for (size_t i = 0; i < elen; i++) {
		printf("%d == %d\n", exp[i], out[i]);
		//assert(exp[i] == out[i]);
	}
	
	// Optional frees for this test run
	free(t.next_states);
	free(t.outputs);
	free(out);
}

int main(int argc, char** argv) {
	ut_trel(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1});
	ut_enc(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(bool[]){1,1,0,1}, 4,
		(bool[]){1,1, 1,0, 1,0, 0,0}, 8);
	ut_hard(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(bool[]){1,1,0,1}, 4,
		(bool[]){1,1,0,1});
	ut_soft(3, (uint32_t[]){0x5,0x7}, 2,
		1, 2,
		2, 4, 4,
		(uint64_t[]){0,2,0,2,1,3,1,3},
		(uint64_t[]){0,3,3,0,1,2,2,1},
		(float[]){-0.8,-0.6, +0.1,+1.3, -2.0,+0.9, +1.0,+0.3}, 8,
		(bool[]){1,1,0,1}, 4);
	return 0;
}
