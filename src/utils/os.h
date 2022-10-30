/* date = October 23rd 2022 1:52 pm */

#ifndef OS_H
#define OS_H

#include "common.h"
#include "containers/string.h"


internal bool os_folder_create_c_str(const char* path);
internal bool os_folder_create_str(String* path);

internal bool os_folder_delete_c_str(const char* path);
internal bool os_folder_delete_str(String* path);

internal bool os_folder_exists_c_str(const char* path);
internal bool os_folder_exists_str(String* path);


#endif //OS_H
