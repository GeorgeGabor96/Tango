/* date = September 16th 2022 11:12 pm */

#ifndef MATH_OPS_H
#define MATH_OPS_H

#include <math.h>
#include "common.h"


inline internal f64
math_op_exp(f64 value) {
    f64 result = exp(value);
    return result;
}


inline internal u32
math_min_u32(u32 a, u32 b) {
    u32 result = a < b ? a : b;
    return result;
}


inline internal u32
math_max_u32(u32 a, u32 b) {
    u32 result = a < b ? b : a;
    return result;
}


#endif //MATH_OPS_H
