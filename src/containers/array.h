#ifndef __CONTAINERS_ARRAY_H__
#define __CONTAINERS_ARRAY_H__

#include "common.h"
#include "utils/memory.h"


// NOTE: this does not have any ordering properties, its just some slots that
// NOTE: are filled or not
typedef struct Array {
    u32 length;
    size_t el_size;
    u8* data;           // NOTE: This should point directly after the Array
} Array;


#define array_data_size(a) ((a)->length * (a)->el_size)
#define array_el_adr(a, i) ((a)->data + (i) * (a)->el_size)

internal Array* array_create(u32 length, size_t el_size);
internal void array_init(Array* array, u32 length, size_t el_size);

internal void array_destroy(Array* array, ResetFn* reset_fn);
internal void array_reset(Array* array, ResetFn* reset_fn);

internal Array* array_resize(Array* array, u32 new_length);

internal void array_set(Array* array, void* element, u32 idx);
internal void* array_get(Array* array, u32 idx);

internal void array_show(Array* array, ShowFn* show_fn);

internal void array_swap(Array* array, u32 idx1, u32 idx2);

// NOTE: this should be usefull when I need to move an array into some other memory
internal size_t array_size(Array* array);
internal size_t array_size_from_params(u32 length, size_t el_size);

#endif // __CONTAINERS_ARRAY_H__
