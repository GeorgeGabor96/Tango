internal u32
_string_get_c_str_length(const char* c_str) {
    u32 length = 0;
    while (*c_str) {
        ++length;
        ++c_str;
    }
    return length;
}

#define str_size_from_length(length) (sizeof(String) + length + 1)
#define str_size(str) str_size_from_length((str)->length)


internal String*
_string_create_from_length(Memory* memory, u32 length) {
    String* str = NULL;
    check(memory != NULL, "memory is NULL");
    check(length > 0, "length is 0");

    str = (String*) memory_push(memory, str_size_from_length(length));
    check_memory(str);
    str->length = length;
    str->data = (char*)(str + 1);
    error:
    return str;
}


internal String*
string_create(Memory* memory, const char* c_str) {
    String* str = NULL;
    check(memory != NULL, "memory is NULL");
    check(c_str != NULL, "c_str is NULL");

    u32 length = _string_get_c_str_length(c_str);
    str = _string_create_from_length(memory, length);
    check_memory(str);
    memcpy(str->data, c_str, str->length + 1);

    error:
    return str;
}


internal b32
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


internal b32
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
string_path_join(Memory* memory, String* str1, String* str2) {
    String* str = NULL;
    check(memory != NULL, "memory is NULL");
    check(str1 != NULL, "str1 is NULL");
    check(str2 != NULL, "str2 is NULL");

    // NOTE: 2 because of os sep and \0
    u32 length = str1->length + 2 + str2->length;
    str = _string_create_from_length(memory, length);
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
string_path_join_c_str(Memory* memory, String* str, const char* c_str) {
    String* result = NULL;
    check(memory != NULL, "memory is NULL");
    check(str != NULL, "str1 is NULL");
    check(c_str != NULL, "c_str is NULL");

    u32 c_str_len = _string_get_c_str_length(c_str);
    u32 length = str->length + 2 + c_str_len;
    result = _string_create_from_length(memory, length);
    u32 str_offset = 0;
    u32 i = 0;
    for (i = 0; i < str->length; ++i, ++str_offset)
        result->data[str_offset] = str->data[i];
    result->data[str_offset++] = STR_OS_PATH_SEP;
    for (i = 0; i < c_str_len; ++i, ++str_offset)
        result->data[str_offset] = c_str[i];
    result->data[str_offset] = '\0';

    error:
    return result;
}


internal const char*
string_get_c_str(String* str) {
    check(str != NULL, "str is NULL");

    return (const char*)str->data;

    error:
    return NULL;
}
