#include "common.h"
#include "tests.h"
#include "containers/list.h"
#include "containers/list.c"


internal TestStatus
list_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    List* list = NULL;
    size_t el_size = sizeof(i16);
    
    list = list_create(el_size);
    assert(list != NULL, "List is NULL");
    assert(list->first == NULL, "list->first should be NULL, not %p",
           list->first);
    assert(list->last == NULL, "list->last should be NULL, not %p",
           list->last);
    assert(list->el_size == el_size, "list->el_size should be %zu, not %zu",
           el_size, list->el_size);
    
    list_destroy(list, NULL);
    
    // EDGE cases
    list = list_create(0);
    assert(list == NULL, "Should be NULL for el_size of 0");
    list_destroy(NULL, NULL);
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
list_add_remove_test() {
    TestStatus status = TEST_FAILED;
    List* list = list_create(sizeof(u32));
    u32 val1 = 1234;
    u32 val2 = 8923;
    u32 val3 = 789345;
    u32 val4 = 78934;
    u32 val = 0;
    
    list_add_first(list, &val1);
    list_add_first(list, &val2);
    list_add_first(list, &val3);
    list_add_first(list, &val4);
    assert(list_is_empty(list) == 0, "List should not be empty");
    list_remove_last(list, &val);
    assert(val == val1, "val should be %u not %u", val1, val);
    list_remove_last(list, &val);
    assert(val == val2, "val should be %u not %u", val2, val);
    list_remove_last(list, &val);
    assert(val == val3, "val should be %u not %u", val3, val);
    list_remove_last(list, &val);
    assert(val == val4, "val should be %u not %u", val4, val);
    assert(list_is_empty(list) != 0, "list should be empty");
    
    list_add_last(list, &val1);
    list_add_last(list, &val2);
    list_add_last(list, &val3);
    list_add_last(list, &val4);
    list_remove_first(list, &val);
    assert(val == val1, "val should be %u not %u", val1, val);
    list_remove_first(list, &val);
    assert(val == val2, "val should be %u not %u", val2, val);
    list_remove_first(list, &val);
    assert(val == val3, "val should be %u not %u", val3, val);
    list_remove_first(list, &val);
    assert(val == val4, "val should be %u not %u", val4, val);
    assert(list_is_empty(list) != 0, "list should be empty");
    
    // EDGE cases
    list_add_first(NULL, &val);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_add_first(list, NULL);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_add_last(NULL, &val);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_add_last(list, NULL);
    assert(list_is_empty(list) != 0, "list should be empty");
    
    list_remove_first(NULL, &val);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_remove_first(list, NULL);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_remove_last(NULL, &val);
    assert(list_is_empty(list) != 0, "list should be empty");
    list_remove_last(list, NULL);
    assert(list_is_empty(list) != 0, "list should be empty");
    
    status = TEST_SUCCESS;
    error:
    list_destroy(list, NULL);
    return status;
}


internal TestStatus
list_show_test() {
    TestStatus status = TEST_FAILED;
    List* list = NULL;
    u32 val1 = 1;
    u32 val2 = 2;
    u32 val3 = 3;
    u32 val4 = 4;
    u32 val5 = 5;
    
    list = list_create(sizeof(u32));
    list_add_last(list, &val1);
    list_add_last(list, &val2);
    list_add_last(list, &val3);
    list_add_last(list, &val4);
    list_add_last(list, &val5);
    
    list_show(list, u32_show);
    
    // edge cases
    list_show(NULL, u32_show);
    list_show(list, NULL);
    
    status = TEST_SUCCESS;
    error:
    list_destroy(list, NULL);
    return status;
}