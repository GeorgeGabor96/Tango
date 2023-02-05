#include "utils/random.h"


// NOTE: https://en.wikipedia.org/wiki/Xorshift
/* The state must be initialized to non-zero */
u32 _xor_shift_32(Random* random) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    u32 x = random->state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
    random->state = x;
	return x;
}


internal Random*
random_create(Memory* memory, u32 seed) {
    check(memory != NULL, "memory is NULL");
    Random* random = (Random*)memory_push(memory, sizeof(*random));
    check_memory(random);

    random->seed = seed;
    random->state = seed;

    return random;
    error:
    return NULL;
}


internal b32
random_get_bool(Random* random, f32 chance) {
    check(random != NULL, "random is NULL");

    b32 result = random_get_chance_f64(random) <= (f64)chance ? TRUE: FALSE;
    return result;

    error:
    return FALSE;
}


internal f64
random_get_chance_f64(Random* random) {
    check(random != NULL, "random is NULL");

    f64 chance = ((f64)_xor_shift_32(random) / (f64)U32_MAX);
    return chance;

    error:
    return 0.0;
}


internal f32
random_get_chance_f32(Random* random) {
    check(random != NULL, "random is NULL");

    f32 chance = (f32)random_get_chance_f64(random);
    return chance;

    error:
    return 0.0f;
}
