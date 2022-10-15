#include "containers/array.h"


internal void*
array_resize(void* array, sz el_size, u32 current_len, u32 new_len) {
    check(array != NULL, "array is NULL");
    check(el_size != 0, "el_size is 0");
    check(current_len != 0, "current_len is 0");
    check(new_len != 0, "new_len is 0");
    
    if (current_len == new_len)
        return array;
    
    void* new_array = memory_malloc(new_len * el_size, "array_resize");
    check_memory(new_array);
    
    u32 elements_to_copy = math_min_u32(current_len, new_len);
    memcpy(new_array, array, elements_to_copy * el_size);
    memset(array, 0, current_len * el_size);
    memory_free(array);
    
    return new_array;
    
    error:
    return NULL;
}