#include "common.h"
#include "tests.h"
#include "containers/string.h"
#include "containers/string.c"
#include "string.h"


internal TestStatus
string_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    const char* c_str = "Ana has big apples";
    String* str = NULL;
    char c = 0;
    
    str = string_create(c_str);
    assert(str != NULL, "str is NULL");
    u32 actual_length = (u32)strlen(c_str);
    assert(actual_length == str->length,
           "actual length is %u not %u",
           actual_length, str->length);
    for (u32 i = 0; i < actual_length; ++i) {
        c = string_char_at_idx(str, i);
        assert(c_str[i] == c,
               "Character should be %c not %c",
               c_str[i], c);
    }
    assert(str->data[actual_length] == '\0',
           "Should have a 0 at the end of str");
    
    // EDGE cases
    String* str2 = string_create(NULL);
    assert(str2 == NULL, "string should be NULL for NULL c string");
    c = string_char_at_idx(NULL, 0);
    assert(c == (char)255, "Should return invalid value for NULL string, not %d", c);
    c = string_char_at_idx(str, actual_length);
    assert(c == (char)255, "Should return invalid value for out of bounds index");
    string_destroy(NULL);
    
    string_destroy(str);
    assert(memory_leak() == 0, "Memory Leak");
    
    status = TEST_SUCCESS;
    
    error:
    return status;
}


internal TestStatus
string_path_join_test() {
    TestStatus status = TEST_FAILED;
    String* str1 = string_create("Ana has big apples");
    String* str2 = string_create("And some big lemons");
    u32 actual_length = 0;
    u32 join_i = 0;
    u32 i = 0;
    char c = 0;
    char c_join = 0;
    String* str_joined = NULL;
    String* str_joined2 = NULL;
    
    str_joined = string_path_join(str1, str2, TRUE);
    assert(str_joined != NULL, "joined string is NULL");
    actual_length = str1->length + 1 + str2->length;
    assert(str_joined->length == actual_length,
           "joined string length should be %u not %u",
           actual_length, str_joined->length);
    for (i = 0; i < str1->length; ++i, ++join_i) {
        c = string_char_at_idx(str1, i);
        c_join = string_char_at_idx(str_joined, join_i);
        assert(c == c_join, "character at position %u should be %d not %d",
               join_i, c, c_join);
    }
    c_join = string_char_at_idx(str_joined, join_i);
    assert(c_join == STR_OS_PATH_SEP,
           "character at position %u should be %u not %u",
           join_i, STR_OS_PATH_SEP, c_join);
    join_i++;
    
    for (i = 0; i < str2->length; ++i, ++join_i) {
        c = string_char_at_idx(str2, i);
        c_join = string_char_at_idx(str_joined, join_i);
        assert(c == c_join, "character at position %u should be %d not %d",
               join_i, c, c_join);
    }
    c_join = str_joined->data[str_joined->length];
    printf("%d", c_join);
    assert(c_join == '\0', "should have a 0 at the end");
    
    // EDGE cases
    str_joined2 = string_path_join(NULL, str2, TRUE);
    assert(str_joined2 == NULL, "should be NULL for NULL str1");
    str_joined2 = string_path_join(str1, NULL, TRUE);
    assert(str_joined2 == NULL, "should be NULL for NULL str2");
    
    // NOTE: check that the strings are destroy with FALSE
    str_joined2 = string_path_join(str1, str2, FALSE);
    assert(str_joined2 != NULL, "joined string is NULL");
    assert(str_joined2->length == str_joined->length,
           "str_joined2->length should be %u not %u",
           str_joined->length, str_joined2->length);
    for (i = 0; i < str_joined2->length; ++i) {
        c = string_char_at_idx(str_joined, i);
        c_join = string_char_at_idx(str_joined2, i);
        assert(c == c_join, "charated at postion %u should be %d not %d",
               i, c, c_join);
    }
    c_join = str_joined2->data[str_joined2->length];
    assert(c_join == '\0', "should have a 0 at the end");
    
    string_destroy(str_joined);
    string_destroy(str_joined2);
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
    
}


internal TestStatus
string_to_c_str_test() {
    TestStatus status = TEST_FAILED;
    String* str = string_create("Ana has big watermelons");
    char c = 0;
    char* c_str = NULL;
    u32 i = 0;
    
    c_str = (char*) string_to_c_str(str);
    assert(c_str != NULL, "c_str is NULL");
    assert(c_str == str->data, "c_str should have addres %p not %p",
           str->data, c_str);
    
    c_str = string_to_c_str_copy(str);
    assert(c_str != NULL, "c_str is NULL");
    assert(c_str != str->data, "c_str should not be %p", c_str);
    
    for (i = 0; i < str->length; ++i) {
        c = string_char_at_idx(str, i);
        assert(c_str[i] == c, "character at index %u should be %d not %d",
               i, c, c_str[i]);
    }
    assert(c_str[str->length] == '\0',
           "c string should be terminated with 0");
    memory_free(c_str);
    
    // EDGE cases
    c_str = (char*) string_to_c_str(NULL);
    assert(c_str == NULL, "c_str should be NULL for invalid string");
    c_str = string_to_c_str_copy(NULL);
    assert(c_str == NULL, "c_str should be NULL for invalid string");
    
    string_destroy(str);
    
    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}