/* date = November 6th 2022 3:23 pm */

#ifndef RANDOM_H
#define RANDOM_H


#include "common.h"
#include "time.h"


internal void random_init();
internal bool random_get_bool(f32 chance);
internal f32 random_get_chance_f32();
internal f64 random_get_chance_f64();


#endif //RANDOM_H
