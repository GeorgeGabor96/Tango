#include "containers/string.h"


inline internal u32
get_c_str_length(const char* c_str) {
    u32 length = 0;
    while (*c_str) {
        ++length;
        ++c_str;
    }
    return length;
}

#define str_size_from_length(length) (sizeof(String) + length + 1)
#define str_size(str) str_size_from_length((str)->length)


inline internal String*
string_create_from_length(MemoryArena* arena, u32 length) {
    String* str = NULL;
    check(arena != NULL, "arena is NULL");
    check(length > 0, "length is 0");
    
    str = (String*) memory_arena_push(arena, str_size_from_length(length));
    check_memory(str);
    str->length = length;
    str->data = (char*)(str + 1);
    error:
    return str;
}


internal String*
string_create(MemoryArena* arena, const char* c_str) {
    String* str = NULL;
    check(arena != NULL, "arena is NULL");
    check(c_str != NULL, "c_str is NULL");
    
    u32 length = get_c_str_length(c_str);
    str = string_create_from_length(arena, length);
    check_memory(str);
    memcpy(str->data, c_str, str->length + 1);
    
    error:
    return str;
}


internal char
string_char_at_idx(String* str, u32 idx) {
    check(str != NULL, "str is NULL");
    check(str->length > idx, "idx is too big");
    
    return str->data[idx];
    
    error:
    return 255;  // NOTE: what value for invalid char??
}


internal bool
string_equal(String* str1, String* str2) {
    u32 i = 0;
    check(str1 != NULL, "str1 is NULL");
    check(str2 != NULL, "str2 is NULL");
    
    if (str1->length != str2->length) return FALSE;
    for (i = 0; i < str1->length; ++i) {
        if (str1->data[i] != str2->data[i]) return FALSE;
    }
    return TRUE;
    
    error:
    return FALSE;
}


internal bool
string_equal_c_str(String* str, const char* c_str) {
    check(str != NULL, "str is NULL");
    check(c_str != NULL, "c_str is NULL");
    
    u32 i = 0;
    for (i = 0; i < str->length && c_str != NULL && *c_str != '\0'; ++i, ++c_str)
        if (str->data[i] != *c_str) return FALSE;
    
    // NOTE: should have compare the whole string
    if (i != str->length) return FALSE;
    // NOTE: should have compare the whole c string
    if (*c_str != '\0') return FALSE;
    
    return TRUE;
    error:
    return FALSE;
}



internal String*
string_path_join(MemoryArena* arena, String* str1, String* str2) {
    String* str = NULL;
    check(arena != NULL, "arena is NULL");
    check(str1 != NULL, "str1 is NULL");
    check(str2 != NULL, "str2 is NULL");
    
    // TODO: nu trebuia + 2?? gen pentru \0
    u32 length = str1->length + 1 + str2->length;
    str = string_create_from_length(arena, length);
    u32 str_offset = 0;
    u32 i = 0;
    for (i = 0; i < str1->length; ++i, ++str_offset) {
        str->data[str_offset] = str1->data[i];
    }
    str->data[str_offset++] = STR_OS_PATH_SEP;
    for (i = 0; i < str2->length; ++i, ++str_offset) {
        str->data[str_offset] = str2->data[i];
    } 
    str->data[str_offset] = '\0';
    
    error:
    return str;
}


internal String*
string_path_join_c_str(MemoryArena* arena, String* str, const char* c_str) {
    check(arena != NULL, "arena is NULL");
    check(str != NULL, "str1 is NULL");
    check(c_str != NULL, "c_str is NULL");
    
    u32 c_str_len = get_c_str_length(c_str);
    u32 length = str->length + 1 + c_str_len;
    str = string_create_from_length(arena, length);
    u32 str_offset = 0;
    u32 i = 0;
    for (i = 0; i < str->length; ++i, ++str_offset)
        str->data[str_offset] = str->data[i];
    str->data[str_offset++] = STR_OS_PATH_SEP;
    for (i = 0; i < c_str_len; ++i, ++str_offset)
        str->data[str_offset] = c_str[i];
    str->data[str_offset] = '\0';
    
    error:
    return str;
}


internal const char*
string_to_c_str(String* str) {
    check(str != NULL, "str is NULL");
    
    return (const char*)str->data;
    
    error:
    return NULL;
}
