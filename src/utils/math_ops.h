#ifndef __UTILS_MATH_OPS_H__
#define __UTILS_MATH_OPS_H__

#include <math.h>


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


inline internal u32
math_clip_u32(u32 value, u32 min, u32 max) {
    u32 result = value;
    
    if (result > max) result = max;
    else if (result < min) result = min;
    
    return result;
}


#endif // __UTILS_MATH_OPS_H__
