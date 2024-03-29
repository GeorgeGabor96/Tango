#ifndef __UTILS_MATH_OPS_H__
#define __UTILS_MATH_OPS_H__

#include <math.h>

#define FLOAT_EPSILON 0.000001f

inline internal b32
math_float_equals_f32(f32 value1, f32 value2) {
    if (value1 <= value2 + FLOAT_EPSILON && value1 >= value2 - FLOAT_EPSILON) {
        return TRUE;
    }
    return FALSE;
}


inline internal u32
math_abs_i32(i32 value) {
    u32 result = value >= 0 ? (u32)value : (u32)(-value);
    return result;
}

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
