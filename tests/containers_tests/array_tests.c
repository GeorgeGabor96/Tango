#include "common.h"
#include "tests.h"
#include "containers/array.h"
#include "containers/array.c"


internal TestStatus
array_create_destroy_test() {
    Array* array = NULL;
    TestStatus status = TEST_FAILED;
    u32 capacity = 256;
    size_t el_size = 128;
    
    array = array_create(capacity, el_size);
    
    assert(array != NULL, "array is NULL");
    assert(array->capacity == capacity,
           "array->capacity is %u, it should be %u",
           array->capacity, capacity);
    assert(array->length == 0, 
           "array->length is %u, it should be 0", array->length);
    assert(array->el_size == el_size,
           "array->el_size is %zu, it should be %zu", array->el_size, el_size);
    assert((void*)(array->data) == (void*)(array + 1), "array->data is %p, it should be dirrectly after the structure, so array is at %p, array->data should point to %p", array->data, array, array + 1);
    
    array_destroy(array, NULL);
    array_destroy(NULL, NULL);
    
    assert(memory_leak() == 0, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_set_get_test() {
    TestStatus status = TEST_FAILED;
    Array* array = NULL;
    u32 capacity = 5;
    size_t el_size = sizeof(u32);
    u32 val0 = 128;
    u32 val1 = 68;
    u32 val2 = 91;
    u32 val3 = 892;
    u32 val4 = 123124;
    u32 val = 0;
    void* val_p = NULL;
    
    array = array_create(capacity, el_size);
    array_set_length(array, 0);
    
    // NORMAL cases
    array_set(array, &val0, 0);
    array_set(array, &val1, 1);
    array_set(array, &val2, 2);
    array_set(array, &val3, 3);
    array_set(array, &val4, 4);
    
    val = *(u32*)array_get(array, 0);
    assert(val == val0, "Value at index 0 is incorrect, real: %u, current: %u", val0, val);
    val = *(u32*)array_get(array, 1);
    assert(val == val1, "Value at index 1 is incorrect, real: %u, current: %u", val1, val);
    val = *(u32*)array_get(array, 2);
    assert(val == val2, "Value at index 2 is incorrect, real: %u, current: %u", val2, val);
    val = *(u32*)array_get(array, 3);
    assert(val == val3, "Value at index 3 is incorrect, real: %u, current: %u", val3, val);
    val = *(u32*)array_get(array, 4);
    assert(val == val4, "Value at index 4 is incorrect, real: %u, current: %u", val4, val);
    
    // EDGE cases
    array_set(array, &val, 5);
    array_set(array, &val, 100);
    array_set(NULL, &val, 0);
    array_set(array, NULL, 0);
    
    val_p = array_get(array, 5);
    assert(val_p == NULL, "Should get NULL for index 5");
    val_p = array_get(array, 100);
    assert(val_p == NULL, "Should get NULL for index 100");
    val_p = array_get(NULL, 0);
    assert(val_p == NULL, "Should get NULL for NULL array");
    
    array_destroy(array, NULL);
    assert(memory_leak() == 0, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_swap_test() {
    TestStatus status = TEST_FAILED;
    Array* array = NULL;
    u32 val1 = 98273;
    u32 val1_i = 17;
    u32 val2 = 78923;
    u32 val2_i = 28;
    u32 val = 0;
    
    // NORMAL cases
    array = array_create(32, sizeof(u32));
    array_set_length(array, 0);
    array_set(array, &val1, val1_i);
    array_set(array, &val2, val2_i);
    
    array_swap(array, val1_i, val2_i);
    val = *(u32*)array_get(array, val1_i);
    assert(val == val2, "Value at index %u should be %u not %u", val1_i, val2, val);
    val = *(u32*)array_get(array, val2_i);
    assert(val == val1, "Value at index %u should be %u not %u", val2_i, val1, val);
    
    array_swap(array, val1_i, val2_i);
    val = *(u32*)array_get(array, val1_i);
    assert(val == val1, "Value at index %u should be %u not %u", val1_i, val1, val);
    val = *(u32*)array_get(array, val2_i);
    assert(val == val2, "Value at index %u should be %u not %u", val2_i, val2, val);
    
    // EDGE cases
    array_swap(array, val1_i, 100);
    val = *(u32*)array_get(array, val1_i);
    assert(val == val1, "Value at index %u should be %u not %u", val1_i, val1, val);
    
    array_swap(array, 100, val1_i);
    val = *(u32*)array_get(array, val1_i);
    assert(val == val1, "Value at index %u should be %u not %u", val1_i, val1, val);
    
    array_swap(array, 100, 200);
    array_swap(NULL, val1_i, val2_i);
    
    array_destroy(array, NULL);
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    
    error:
    return status;
}


internal TestStatus
array_show_test() {
    TestStatus status = TEST_FAILED;
    Array* array = array_create(3, sizeof(i32));
    array_set_length(array, 0);
    i32 val1 = 1;
    array_set(array, &val1, 0);
    i32 val2 = 2;
    array_set(array, &val2, 1);
    i32 val3 = 3;
    array_set(array, &val3, 2);
    i32 val = 0;
    
    array_show(array, i32_show);
    
    val = *(i32*)array_get(array, 0);
    assert(val == val1, "Value at idx: 0 should be %d not %d", val1, val);
    val = *(i32*)array_get(array, 1);
    assert(val == val2, "Value at idx: 1 should be %d not %d", val2, val);
    val = *(i32*)array_get(array, 2);
    assert(val == val3, "Value at idx: 2 should be %d not %d", val3, val);
    
    // EDGE cases
    array_show(NULL, i32_show);
    array_show(array, NULL);
    
    array_destroy(array, NULL);
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    
    error:
    return status;
}


internal TestStatus
array_size_test() {
    TestStatus status = TEST_FAILED;
    u32 capacity = 10;
    size_t el_size = sizeof(f64);
    Array* array = array_create(capacity, el_size);
    size_t actual_size = sizeof(Array) + capacity * el_size;
    
    size_t size_of_array = array_size(array);
    assert(size_of_array == actual_size, "Actual size: %zu, Computed size; %zu", actual_size, size_of_array);
    size_of_array = array_size(NULL);
    assert(0 == size_of_array, "Size for NULL is 0 not: %zu", size_of_array);
    
    array_destroy(array, NULL);
    assert(memory_leak() == 0, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_set_length_test() {
    TestStatus status = TEST_FAILED;
    
    Array* array = array_create(32, sizeof(u32));
    
    array_set_length(array, 5);
    assert(array->length == 5, "array->length is %u, it should be 5",
           array->length, 5);
    array_set_length(array, 0);
    assert(array->length == array->capacity,
           "array->length is %u, should be %u",
           array->length, array->capacity);
    
    // EDGE CASES
    array_set_length(NULL, 0);
    array_set_length(array, 3);
    assert(array->length = array->capacity,
           "array->length should not be modified when asking for smaller length");
    
    array_destroy(array, NULL);
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_squeeze_test() {
    TestStatus status = TEST_FAILED;
    
    Array* new_array = NULL;
    Array* array = array_create(32, sizeof(u32));
    array_set_length(array, 5);
    
    new_array = array_squeeze(array);
    assert(new_array != NULL, "new_array is NULL");
    assert(new_array->length == 5, "new_array->lenght is %u, should be 5", 
           new_array->length);
    assert(new_array->capacity == 5, "new_array->capacity is %u, should be 5",
           new_array->capacity);
    assert(new_array != array, "new_array should not be %p", array);
    array_destroy(new_array, NULL);
    
    // EDGE cases
    array = array_squeeze(NULL);
    assert(array == NULL, "array should be NULL for NULL input");
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_increase_capacity_test() {
    TestStatus status = TEST_FAILED;
    
    u32 i = 0;
    Array* array = array_create(32, sizeof(u32));
    array_set_length(array, 0);
    u32 value = 1;
    u32 rec_value = 0;
    for (i = 0; i < 16; ++i) {
        array_set(array, &value, i);
    }
    
    Array* new_array = array_increase_capacity(array, 64);
    array_set_length(new_array, 0);
    assert(new_array->capacity == 64,
           "new_array->capacity is %u, should be 64", new_array->capacity);
    assert(new_array != array,
           "new_array should not have the same value as array");
    for (i = 0; i < 16; ++i) {
        rec_value = *((u32*)array_get(new_array, i));
        assert(rec_value == value, "value at index %u should be %u, not %u",
               i, value, rec_value);
    }
    for (i = 16; i < new_array->capacity; ++i) {
        rec_value = *((u32*)array_get(new_array, i));
        assert(rec_value == 0, "value at index %u should be 0, not %u",
               i, rec_value);
    }
    
    // EDGE cases
    array = array_increase_capacity(NULL, 0);
    assert(array == NULL, "array should be NULL for NULL input");
    array = array_increase_capacity(new_array, 12);
    assert(array == NULL, "array should be NULL for smaller capacity");
    array_destroy(new_array, NULL);
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
array_append_test() {
    TestStatus status = TEST_FAILED;
    
    Array* array = array_create(2, sizeof(u32));
    Array* new_array = array;
    u32 value = 0;
    for (u32 i = 0; i < 10; ++i) {
        new_array = array_append(new_array, &i);
        assert(new_array != NULL, "new_array is NULL");
        value = *((u32*)array_get(new_array, i));
        assert(value == i, "value at idx %u should be %u, not %u",
               i, i, value);
    }
    assert(new_array != array, "new_array should be different from array");
    assert(new_array->length == 10, "length should be 10");
    assert(new_array->capacity == 16, "capacity should be 16, not %u",
           new_array->capacity);
    
    // EDGE CASES
    array = array_append(NULL, &value);
    assert(array == NULL, "array should be NULL for invalid input array");
    array = array_append(new_array, NULL);
    assert(array == NULL, "array should be NULL for invalid element");
    
    array_destroy(new_array, NULL);
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}