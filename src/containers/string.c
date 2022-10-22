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
string_create_from_length(u32 length) {
    String* str = (String*) memory_malloc(str_size_from_length(length),
                                          "string_create_from_length");
    check_memory(str);
    str->length = length;
    str->data = (char*)(str + 1);
    error:
    return str;
}


internal String*
string_create(const char* c_str) {
    String* str = NULL;
    check(c_str != NULL, "c_str is NULL");
    
    u32 length = get_c_str_length(c_str);
    str = string_create_from_length(length);
    check_memory(str);
    memcpy(str->data, c_str, str->length + 1);
    
    error:
    return str;
}


internal void
string_destroy(String* str) {
    check(str != NULL, "str is NULL");
    memset(str, 0, str_size(str));
    memory_free(str);
    
    error:
    return;
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
string_path_join(String* str1, String* str2, bool keep_strs) {
    String* str = NULL;
    check(str1 != NULL, "str1 is NULL");
    check(str2 != NULL, "str2 is NULL");
    
    u32 length = str1->length + 1 + str2->length;
    str = string_create_from_length(length);
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
    
    if (keep_strs == FALSE) {
        string_destroy(str1);
        string_destroy(str2);
    }
    
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


internal char*
string_to_c_str_copy(String* str) {
    char* c_str = NULL;
    check(str != NULL, "str is NULL");
    
    c_str = (char*) memory_malloc((str->length + 1) * sizeof(char),
                                  "string_to_c_str_copy");
    check_memory(c_str);
    
    u32 i = 0;
    for (i = 0; i < str->length; ++i) {
        c_str[i] = str->data[i];
    }
    c_str[i] = '\0';
    
    error:
    return c_str;
}
