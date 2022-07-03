#ifndef __CONTAINERS_ARRAY__
#define __CONTAINERS_ARRAY__

#include "common.h"


typedef struct Array {
    u32 length;
    size_t el_size;
    u8* data;           // NOTE: This should point directly after the Array
} Array;

#define array_data_size(a) ((a)->length * (a)->el_size)
#define array_el_adr(a, i) ((a)->data + (i) * (a)->el_size)

Array* array_create(u32 length, size_t el_size);
void array_init(Array* array, u32 length, size_t el_size);

void array_destroy(Array* array, ResetFn reset);
void array_reset(Array* array, ResetFn reset);

void array_set(Array* array, void* element, u32 idx);
void* array_get(Array* array, u32 idx);

void array_show(Array* array, ShowFn show);

void array_swap(Array* array, u32 idx1, u32 idx2);

#endif
