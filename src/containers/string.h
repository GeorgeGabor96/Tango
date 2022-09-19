/* date = July 23rd 2022 11:09 pm */

#ifndef STRING_H
#define STRING_H

#include "common.h"


// TODO: need to know which os sep to use, how do I know I'm on windows or linux???
//#define STR_OS_PATH_SEP '/'
#define STR_OS_PATH_SEP '\\'


typedef struct String {
    u32 length;
    char* data;   // NOTE: this should point directly after the String
} String;


internal String* string_create(const char* c_str);
internal void* string_destroy(String* str);
internal String* string_path_join(String* str1, String* str2, bool keep_strs);

internal const char* string_to_c_str(String* str);
internal char* string_to_c_str_copy(String* str);

#endif //STRING_H
