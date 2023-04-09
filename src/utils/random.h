#ifndef __UTILS_RANDOM_H__
#define __UTILS_RANDOM_H__

#include "time.h"


typedef struct Random {
    u32 seed;
    u32 state;
} Random;


internal Random* random_create(Memory* memory, u32 seed);
internal b32 random_get_b32(Random* random, f32 chance);
internal b8 random_get_b8(Random* random, f32 chance);
internal f32 random_get_chance_f32(Random* random);
internal f64 random_get_chance_f64(Random* random);


#endif // __UTILS_RANDOM_H__
