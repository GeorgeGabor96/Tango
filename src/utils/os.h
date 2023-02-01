#ifndef __UTILS_OS_H__
#define __UTILS_OS_H__


internal b32 os_folder_create_c_str(const char* path);
internal b32 os_folder_create_str(String* path);

internal b32 os_folder_delete_c_str(const char* path);
internal b32 os_folder_delete_str(String* path);

internal b32 os_folder_exists_c_str(const char* path);
internal b32 os_folder_exists_str(String* path);


#endif // __UTILS_OS_H__
