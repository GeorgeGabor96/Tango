#ifndef __UTILS_MATH_OPS_H__
#define __UTILS_MATH_OPS_H__

#include <math.h>


inline internal f64
math_exp_f64(f64 value) {
    f64 result = exp(value);
    return result;
}

inline internal f32
math_exp_f32(f32 value) {
    return (f32)math_exp_f64(value);
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
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

inline internal f32
math_clip_f32(f32 value, f32 min, f32 max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}


#endif // __UTILS_MATH_OPS_H__
