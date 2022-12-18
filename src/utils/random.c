#include "utils/random.h"


typedef struct RandomState {
    u32 state;
} RandomState;


RandomState random_state_global;


// NOTE: https://en.wikipedia.org/wiki/Xorshift
/* The state must be initialized to non-zero */
u32 xor_shift_32(RandomState* random_state) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    u32 x = random_state->state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
    random_state->state = x;
	return x;
}


internal void
random_init() {
    srand((u32)time(0));
    while (random_state_global.state == 0)
        random_state_global.state = rand();
}


internal bool
random_get_bool(f32 chance) {
    bool result = random_get_chance_f64() <= (f64)chance ? TRUE: FALSE;
    return result;
}


internal f64
random_get_chance_f64() {
    f64 chance = ((f64)xor_shift_32(&random_state_global) / (f64)U32_MAX);
    return chance;
}


internal f32
random_get_chance_f32() {
    f32 chance = (f32)random_get_chance_f64();
    return chance;
}
