#include "containers/array.h"

// TODO: Add checks macros pre and post conditions


Array* array_create(u32 length, size_t el_size) {
    Array* array = NULL;
    
    array = (Array*) malloc(sizeof(array) + el_size * length);
    if (array == NULL) return NULL;

    array_init(array, length, el_size);
    return array;
}


void array_init(Array* array, u32 length, size_t el_size) {
    array->length = length;
    array->el_size = el_size;
    array->data = array + 1;    // data is located after the array structure
    memset(array->data, 0, array_data_size(array));
}


void array_destroy(Array* array, ResetFn reset_fn) {
    array_reset(array, reset_fn);
    free(array);
}


void array_reset(Array* array, ResetFn reset_fn) {
    for (u32 idx = 0; idx < array->length; ++idx) {
        reset_fn(array_get(array, idx);
    }
    memset(array->data, 0, array_data_size(array));
    array->length = 0;
    array->el_size = 0;
    array->data = NULL;
}


void array_set(Array* array, void* element, u32 idx) {
    u8* el_adr = array_el_adr(array, idx);
    memcpy(el_adr, element, array->el_size);
}


void* array_get(Array* array, u32 idx) {
    u8* el_adr = array_el_adr(array, idx);
    return el_adr;
}


void array_show(Array* array, ShowFn show_fn) {
    for (u32 idx = 0; idx < array->length; ++idx) {
        u8* el_adr = array_el_adr(array, idx);
        show_fn(el_adr);
        printf(" ");
    }
    printf("\n");
}


void array_swap(Array* array, u32 idx1, u32 idx2) 
    u8* el1_adr = array_el_adr(array, idx1);
    u8* el2_ard = array_el_adr(array, idx2);
    
    // NOTE: Go over each byte in both the elements and swap byte by byte
    for (size_t adr_idx = 0; adr_idx < array->el_size; ++adr_idx) {
        el1_adr[adr_idx] = el1_adr[adr_idx] ^ el2_adr[adr_idx];
        el2_adr[adr_idx] = el2_adr[adr_idx] ^ el1_adr[adr_idx];
        el1_adr[adr_idx] = el1_adr[adr_idx] ^ el2_adr[adr_idx];
    }
}
