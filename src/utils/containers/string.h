#ifndef __UTILS_CONTAINERS_STRING_H__
#define __UTILS_CONTAINERS_STRING_H__


#ifdef _WIN32
#define STR_OS_PATH_SEP '\\'
#endif


typedef struct String {
    u32 length;   // does not contain the \0 in the length
    char* data;   // does contain the \0
    // NOTE: this should point directly after the String structure
} String;


internal String* string_create(Memory* memory, const char* c_str);

internal b32 string_equal(String* str1, String* str2);
internal b32 string_equal_c_str(String* str, const char* c_str);
internal String* string_path_join(Memory* memory,
                                  String* str1,
                                  String* str2);
internal String* string_path_join_c_str(Memory* memory,
                                        String* str1,
                                        const char* c_str);
internal const char* string_get_c_str(String* str);

#endif // __UTILS_CONTAINERS_STRING_H__
