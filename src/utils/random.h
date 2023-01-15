#ifndef __UTILS_RANDOM_H__
#define __UTILS_RANDOM_H__

#include "time.h"


internal void random_init();
internal bool random_get_bool(f32 chance);
internal f32 random_get_chance_f32();
internal f64 random_get_chance_f64();


#endif // __UTILS_RANDOM_H__
