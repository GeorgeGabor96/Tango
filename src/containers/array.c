#include "containers/array.h"


internal Array*
array_create(u32 length, size_t el_size) {
    check(length != 0, "length is 0");
    check(el_size != 0, "el_size is 0");
    Array* array = NULL;
    
    size_t size = array_size_from_params(length, el_size);
    array = (Array*) memory_malloc(size, "array_create");
    check_memory(array);
    
    array_init(array, length, el_size);
    error:
    return array;
}


internal void
array_init(Array* array, u32 length, size_t el_size) {
    check(array != NULL, "NULL array");
    array->length = length;
    array->el_size = el_size;
    array->data = (u8*)(array + 1);    // data is located after the array structure
    // TODO: shouldn't always set to 0
    memset(array->data, 0, array_data_size(array));
    error:
    return;
}


internal void
array_destroy(Array* array, ResetFn* reset_fn) {
    check(array != NULL, "NULL array");
    array_reset(array, reset_fn);
    memory_free(array);
    error:
    return;
}


internal void
array_reset(Array* array, ResetFn* reset_fn) {
    check(array != NULL, "NULL array");
    if (reset_fn != NULL) {
        for (u32 idx = 0; idx < array->length; ++idx) {
            reset_fn(array_get(array, idx));
        }
    }
    memset(array->data, 0, array_data_size(array));
    array->length = 0;
    array->el_size = 0;
    array->data = NULL;
    error:
    return;
}


internal Array*
array_resize(Array* array, u32 new_length) {
    // TODO: need to check if it 
    
    // with an array create of new_length and a memcpy depending if we need more or less elements
    
}



internal void
array_set(Array* array, void* element, u32 idx) {
    check(array != NULL, "NULL array");
    check(element != NULL, "NULL element");
    check(idx < array->length, "INVALID idx: %u, length: %u", idx, array->length);
    u8* el_adr = array_el_adr(array, idx);
    memcpy(el_adr, element, array->el_size);
    error:
    return;
}


internal void*
array_get(Array* array, u32 idx) {
    check(array != NULL, "NULL array");
    check(idx < array->length, "INVALID idx: %u, length: %u", idx, array->length);
    u8* el_adr = array_el_adr(array, idx);
    return el_adr;
    error:
    return NULL;
}


internal void
array_show(Array* array, ShowFn* show_fn) {
    check(array != NULL, "NULL array");
    check(show_fn != NULL, "NULL show_fn");
    for (u32 idx = 0; idx < array->length; ++idx) {
        u8* el_adr = array_el_adr(array, idx);
        show_fn(el_adr);
        printf(" ");
    }
    printf("\n");
    error:
    return;
}


internal void
array_swap(Array* array, u32 idx1, u32 idx2) {
    check(array != NULL, "NULL array");
    check(idx1 < array->length, "INVALID idx1: %u, length %u", idx1, array->length);
    check(idx2 < array->length, "INVALID idx2: %u, length %u", idx2, array->length);
    u8* el1_adr = array_el_adr(array, idx1);
    u8* el2_adr = array_el_adr(array, idx2);
    
    // NOTE: Go over each byte in both the elements and swap byte by byte
    for (size_t adr_idx = 0; adr_idx < array->el_size; ++adr_idx) {
        el1_adr[adr_idx] = el1_adr[adr_idx] ^ el2_adr[adr_idx];
        el2_adr[adr_idx] = el2_adr[adr_idx] ^ el1_adr[adr_idx];
        el1_adr[adr_idx] = el1_adr[adr_idx] ^ el2_adr[adr_idx];
    }
    error:
    return;
}


internal size_t
array_size(Array* array) {
    check(array != NULL, "NULL array");
    size_t size = array_size_from_params(array->length, array->el_size);
    return size;
    error:
    return 0;
}

internal size_t
array_size_from_params(u32 length, size_t el_size) {
    size_t size = sizeof(Array) + length * el_size;
    return size;
}