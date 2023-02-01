#include "common.h"
#include "tests.h"
#include "containers/string.h"
#include "containers/string.c"
#include "string.h"


internal TestStatus
string_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1));
    const char* c_str = "Ana has big apples";
    String* str = NULL;
    char c = 0;

    str = string_create(arena, c_str);
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
    String* str2 = string_create(NULL, "Ana are mere");
    assert(str2 == NULL, "string should be NULL for NULL arena");
    str2 = string_create(arena, NULL);
    assert(str2 == NULL, "string should be NULL for NULL c string");
    c = string_char_at_idx(NULL, 0);
    assert(c == (char)255, "Should return invalid value for NULL string, not %d", c);
    c = string_char_at_idx(str, actual_length);
    assert(c == (char)255, "Should return invalid value for out of bounds index");

    memory_arena_destroy(arena);

    assert(memory_leak() == 0, "Memory Leak");

    status = TEST_SUCCESS;

    error:
    return status;
}


internal TestStatus
string_path_join_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1));
    String* str1 = string_create(arena, "Ana has big apples");
    String* str2 = string_create(arena, "And some big lemons");
    u32 actual_length = 0;
    u32 join_i = 0;
    u32 i = 0;
    char c = 0;
    char c_join = 0;
    String* str_joined = NULL;
    String* str_joined2 = NULL;

    str_joined = string_path_join(arena, str1, str2);
    assert(str_joined != NULL, "joined string is NULL");
    actual_length = str1->length + 2 + str2->length;
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
    str_joined2 = string_path_join(NULL, str1, str2);
    assert(str_joined2 == NULL, "should be NULL for NULL arena");
    str_joined2 = string_path_join(arena, NULL, str2);
    assert(str_joined2 == NULL, "should be NULL for NULL str1");
    str_joined2 = string_path_join(arena, str1, NULL);
    assert(str_joined2 == NULL, "should be NULL for NULL str2");

    memory_arena_destroy(arena);

    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;

}


internal TestStatus
string_to_c_str_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1));

    String* str = string_create(arena, "Ana has big watermelons");
    char c = 0;
    char* c_str = NULL;
    u32 i = 0;

    c_str = (char*) string_to_c_str(str);
    assert(c_str != NULL, "c_str is NULL");
    assert(c_str == str->data, "c_str should have addres %p not %p",
           str->data, c_str);

    // EDGE cases
    c_str = (char*) string_to_c_str(NULL);
    assert(c_str == NULL, "c_str should be NULL for invalid string");

    memory_arena_destroy(arena);

    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
string_equal_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1));

    String* str1 = string_create(arena, "Ana has big apples");
    String* str2 = string_create(arena, "Ana has big apples");
    b32 is_equal = FALSE;

    is_equal = string_equal(str1, str2);
    assert(is_equal == TRUE, "str1 and str2 should be equal");
    is_equal = string_equal(str2, str1);
    assert(is_equal == TRUE, "str2 and str1 should be equal");

    str2 = string_create(arena, "Ana");
    is_equal = string_equal(str1, str2);
    assert(is_equal == FALSE, "should be false for different lenghts");

    str2 = string_create(arena, "Ana has big appleS");
    is_equal = string_equal(str1, str2);
    assert(is_equal == FALSE, "str1 and str2 are not the same");

    memory_arena_destroy(arena);

    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
string_equal_c_str_test() {
    TestStatus status = TEST_FAILED;

    MemoryArena* arena = memory_arena_create(MB(1));

    String* str = string_create(arena, "Ana has big lemmons");
    const char* c_str = "Ana has big lemmons";
    b32 is_equal = FALSE;

    is_equal = string_equal_c_str(str, c_str);
    assert(is_equal == TRUE, "str and c_str should be equal");

    c_str = "Ana";
    is_equal = string_equal_c_str(str, c_str);
    assert(is_equal == FALSE, "should be false for different lengths");

    c_str = "Ana has big lemmonS";
    is_equal = string_equal_c_str(str, c_str);
    assert(is_equal == FALSE, "should be false for different strings");

    memory_arena_destroy(arena);

    assert(memory_leak() == 0, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}