#include "containers/array.h"


internal Array*
array_create(u32 capacity, size_t el_size) {
    check(capacity != 0, "capacity is 0");
    check(el_size != 0, "el_size is 0");
    Array* array = NULL;
    
    size_t size = array_size_from_params(capacity, el_size);
    array = (Array*) memory_malloc(size, "array_create");
    check_memory(array);
    
    array_init(array, capacity, el_size);
    error:
    return array;
}


internal void
array_init(Array* array, u32 capacity, size_t el_size) {
    check(array != NULL, "NULL array");
    array->capacity = capacity;
    array->length = 0;
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
    memset(array, 0, array_size(array));
    error:
    return;
}


internal Array*
array_increase_capacity(Array* array, u32 capacity) {
    check(array != NULL, "array is NULL");
    check(capacity > array->capacity, "capacity is too small");
    
    Array* new_array = array_create(capacity, array->el_size);
    check_memory(new_array);
    
    memcpy(new_array->data, array->data, array_data_size(array));
    new_array->length = array->length;
    
    array_destroy(array, NULL);
    
    return new_array;
    
    error:
    return NULL;
}


internal Array*
array_squeeze(Array* array) {
    check(array != NULL, "array is NULL");
    
    Array* new_array = array_create(array->length, array->el_size);
    check_memory(new_array);
    
    memcpy(new_array->data, array->data, array->length * array->el_size);
    new_array->length = array->length;
    
    array_destroy(array, NULL);
    
    return new_array;
    
    error:
    return NULL;
}


internal Array*
array_append(Array* array, void* element) {
    check(array != NULL, "array is NULL");
    check(element != NULL, "element is NULL");
    
    Array* new_array = array;
    if (new_array->length == new_array->capacity) {
        new_array = array_increase_capacity(array, array->capacity * 2);
        check_memory(new_array);
    }
    
    ++(new_array->length);
    array_set(new_array, element, new_array->length - 1);
    return new_array;
    
    error:
    return NULL;
}


internal void
array_set_length(Array* array, u32 length) {
    check(array != NULL, "array is NULL");
    check(length <= array->capacity, "length is too big");
    
    if (length == 0) {
        array->length = array->capacity;
    } else if (length > array->length) {
        array->length = length;
    }
    
    error:
    return;
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
    size_t size = array_size_from_params(array->capacity, array->el_size);
    return size;
    error:
    return 0;
}

internal size_t
array_size_from_params(u32 capacity, size_t el_size) {
    size_t size = sizeof(Array) + capacity * el_size;
    return size;
}