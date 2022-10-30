#include "array.h"


internal void*
array_increase_length(void* array, sz el_size, u32 length, u32 new_length) {
    check(array != NULL, "array is NULL");
    check(length < new_length, "current length should be smaller than the new_length");
    check(el_size != 0, "el_size is 0");
    
    void* new_array = (void*) memory_calloc(new_length, el_size);
    check_memory(new_array);
    sz array_size = el_size * length;
    
    memcpy(new_array, array, array_size);
    memset(array, 0, array_size);
    memory_free(array);
    
    return new_array;
    
    error:
    return NULL;
}


