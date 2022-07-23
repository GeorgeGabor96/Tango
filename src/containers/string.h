/* date = July 23rd 2022 11:09 pm */

#ifndef STRING_H
#define STRING_H

#include "common.h"


typedef struct String {
    u32 length;
    u8* data;   // NOTE: this should point directly after the String
} String;

#endif //STRING_H
