/* date = July 23rd 2022 11:09 pm */

#ifndef STRING_H
#define STRING_H

#include "common.h"


// TODO: need to know which os sep to use, how do I know I'm on windows or linux???
//#define STR_OS_PATH_SEP '/'

#ifdef _WIN32
#define STR_OS_PATH_SEP '\\'
#endif


typedef struct String {
    u32 length;
    char* data;   // NOTE: this should point directly after the String
} String;


internal String* string_create(const char* c_str);
internal void string_destroy(String* str);
internal char string_char_at_idx(String* str, u32 idx);

internal bool string_equal(String* str1, String* str2);
internal bool string_eqaul_c_str(String* str, const char* c_str);
internal String* string_path_join(String* str1, String* str2, bool keep_strs);
internal String* string_path_join_c_str(String* str1, const char* c_str, bool keep_str1);

internal const char* string_to_c_str(String* str);
internal char* string_to_c_str_copy(String* str);

#endif //STRING_H
