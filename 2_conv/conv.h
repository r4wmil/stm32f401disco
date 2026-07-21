#ifndef CONV_H_
#define CONV_H_

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct conv_trel_t {
	size_t constr_len;   // constraint length
	size_t input_len;   // input bits
	size_t output_len;   // output bits
	size_t input_syms;  // number of input symbols
	size_t output_syms;  // number of output symbols
	size_t states_num;   // number of states
	uint64_t* next_states; // next states
	uint64_t* outputs; // outputs
} conv_trel_t;

conv_trel_t conv_trel_gen(size_t constr_len, uint32_t* G, size_t output_len);
size_t conv_get_enc_len(conv_trel_t trel, size_t ilen);
void conv_enc(conv_trel_t trel, bool* inp, size_t ilen, bool* out);
void conv_hard(conv_trel_t trel, bool* inp, size_t ilen, bool* out);
void conv_soft(conv_trel_t trel, float* inp, size_t ilen, bool* out);

#endif /* CONV_H_ */

#ifdef CONV_IMPLEMENTATION

conv_trel_t conv_trel_gen(size_t constr_len, uint32_t* G, size_t output_len) {
	conv_trel_t t = {0};
	t.constr_len = constr_len;
	t.input_len = 1; // TODO: 2D
	t.output_len = output_len;
	t.input_syms = 0x1 << t.input_len;
	t.output_syms = 0x1 << t.output_len;
	t.states_num = 0x1 << ((t.constr_len - 1) * t.input_len);
	t.next_states = calloc(t.states_num * t.input_syms, sizeof(*t.next_states));
	t.outputs = calloc(t.states_num * t.input_syms, sizeof(*t.outputs));
	for (size_t s = 0; s < t.states_num; s++) {
		for (size_t i = 0; i < t.input_syms; i++) {
			t.next_states[s * t.input_syms + i] = (s >> t.input_len) | (i << ((t.constr_len - 2) * t.input_len));
			uint64_t r = s | (i << ((t.constr_len - 2) * t.input_len + 1));
			for (size_t g = 0; g < output_len; g++) {
				t.outputs[s * t.input_syms + i] <<= 1;
				t.outputs[s * t.input_syms + i] |= __builtin_parity(r & G[g]);
			}
		}
	}
	return t;
}

size_t conv_get_enc_len(conv_trel_t trel, size_t ilen) {
	return ilen * trel.output_len;
}

void conv_enc(conv_trel_t trel, bool* inp, size_t ilen, bool* out) {
	size_t state = 0;
	size_t outi = 0;
	for (size_t i = 0; i < ilen; i++) {
		size_t tbli = state * trel.input_syms + inp[i];
		uint64_t next_output = trel.outputs[tbli];
		for (size_t b = 0; b < trel.output_len; b++) {
			out[outi++] = (next_output >> (trel.output_len - b - 1)) & 0x1;
		}
		state = trel.next_states[tbli];
	}
}

const uint32_t INF = 1e9;

void conv_hard(conv_trel_t trel, bool* inp, size_t ilen, bool* out) {
    size_t steps = ilen / trel.output_len;

    size_t *path = malloc(steps * trel.states_num * sizeof(*path));
    uint32_t *metrics = malloc((steps + 1) * trel.states_num * sizeof(*metrics));

    metrics[0] = 0;
    for (size_t s = 1; s < trel.states_num; s++) {
        metrics[s] = INF;
    }

    for (size_t t = 0; t < steps; t++) {
        uint32_t *curr_metr = metrics + t * trel.states_num;
        uint32_t *next_metr = metrics + (t + 1) * trel.states_num;

        for (size_t s = 0; s < trel.states_num; s++) {
            next_metr[s] = INF;
        }

        for (size_t s = 0; s < trel.states_num; s++) {
            if (curr_metr[s] == INF)
                continue;

            for (size_t i = 0; i < trel.input_syms; i++) {
                size_t next_id = s * trel.input_syms + i;
                size_t next_s = trel.next_states[next_id];
                uint64_t expected_output = trel.outputs[next_id];

                uint32_t dist = 0;
                for (size_t b = 0; b < trel.output_len; b++) {
                    bool bit = (expected_output >> (trel.output_len - b - 1)) & 0x1;
                    dist += (bit != inp[t * trel.output_len + b]);
                }

                uint32_t cost = curr_metr[s] + dist;

                if (cost < next_metr[next_s]) {
                    next_metr[next_s] = cost;
                    path[t * trel.states_num + next_s] = s;
                }
            }
        }
    }

    // Best final
    uint32_t *last = metrics + steps * trel.states_num;
    size_t best_s = 0;
    for (size_t s = 1; s < trel.states_num; s++) {
        if (last[s] < last[best_s]) { best_s = s; }
    }

    // Traceback
    for (size_t t = steps; t > 0; t--) {
        size_t prev_s = path[(t - 1) * trel.states_num + best_s];

        for (size_t i = 0; i < trel.input_syms; i++) {
            if (trel.next_states[prev_s * trel.input_syms + i] == best_s) {
                out[t - 1] = (bool)i;
                break;
            }
        }

        best_s = prev_s;
    }

    free(path);
    free(metrics);
}

void conv_soft(conv_trel_t trel, float* inp, size_t ilen, bool* out) {
    size_t steps = ilen / trel.output_len;

    size_t *path = malloc(steps * trel.states_num * sizeof(*path));
    float *metrics = malloc((steps + 1) * trel.states_num * sizeof(*metrics));

    const float INF = 1e9f;

    // Initial metrics
    metrics[0] = 0.0f;
    for (size_t s = 1; s < trel.states_num; s++)
        metrics[s] = INF;

    // Forward pass
    for (size_t t = 0; t < steps; t++) {
        float *curr_metr = metrics + t * trel.states_num;
        float *next_metr = metrics + (t + 1) * trel.states_num;

        for (size_t s = 0; s < trel.states_num; s++)
            next_metr[s] = INF;

        for (size_t s = 0; s < trel.states_num; s++) {
            if (curr_metr[s] >= INF)
                continue;

            for (size_t i = 0; i < trel.input_syms; i++) {
                size_t next_id = s * trel.input_syms + i;
                size_t next_s = trel.next_states[next_id];
                uint64_t expected = trel.outputs[next_id];

                float dist = 0.0f;
                for (size_t b = 0; b < trel.output_len; b++) {
                    bool bit = (expected >> (trel.output_len - b - 1)) & 1;
                    float target = bit ? -1.0f : 1.0f;
                    float diff = inp[t * trel.output_len + b] - target;
                    dist += diff * diff;
                }

                float cost = curr_metr[s] + dist;

                if (cost < next_metr[next_s]) {
                    next_metr[next_s] = cost;
                    path[t * trel.states_num + next_s] = s;
                }
            }
        }
    }

    // Best final
    float *last = metrics + steps * trel.states_num;
    size_t best_s = 0;
    for (size_t s = 1; s < trel.states_num; s++)
        if (last[s] < last[best_s])
            best_s = s;

    // Traceback
    for (size_t t = steps; t > 0; t--) {
        size_t prev_s = path[(t - 1) * trel.states_num + best_s];

        for (size_t i = 0; i < trel.input_syms; i++) {
            if (trel.next_states[prev_s * trel.input_syms + i] == best_s) {
                out[t - 1] = (bool)i;
                break;
            }
        }

        best_s = prev_s;
    }

    free(path);
    free(metrics);
}

#endif /* CONV_IMPLEMENTATION */
