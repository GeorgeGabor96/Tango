#include <Windows.h>


internal b32
os_folder_create_c_str(const char* path) {
    check(path != NULL, "path is NULL");

    // Don't create if already exists because CreateDirectory will fail
    if (os_folder_exists_c_str(path) == TRUE) return TRUE;

    SECURITY_ATTRIBUTES attr = { 0 };
    attr.nLength = sizeof(attr);

    b32 result = CreateDirectoryA(path, &attr);

    return result;
    error:
    return FALSE;
}


internal b32
os_folder_create_str(String* path) {
    check(path != NULL, "path is NULL");

    b32 result = os_folder_create_c_str(string_get_c_str(path));
    return result;

    error:
    return FALSE;
}


internal b32
os_folder_delete_c_str(const char* path) {
    check(path != NULL, "path is NULL");

    // NOTE: Just to be save only delete if it exists
    if (os_folder_exists_c_str(path) == FALSE) return TRUE;

    // NOTE: I use SHFileOperationA because its the simplest way to remove everything from
    // NOTE: a folder, but from what I saw it doesn't delete the folder itself
    // NOTE: thats why RemoveDirectoryA is called after

    // NOTE: need to have a double null terminated string
    char path_to_delete[MAX_PATH] = { 0 };
    const char* aux_path = path;
    u32 i = 0;
    while (aux_path[i] != '\0' && i < MAX_PATH - 2) {
        path_to_delete[i] = aux_path[i];
        ++aux_path;
    }
    path_to_delete[i++] = '\0';
    path_to_delete[i] = '\0';

    SHFILEOPSTRUCTA params = { 0 };
    params.wFunc = FO_DELETE;
    params.pFrom = path_to_delete;
    params.fFlags = FOF_NOCONFIRMATION | FOF_NO_UI;

    int result = SHFileOperationA(&params);
    check(result == 0, "The deletion of %s wasn't succesfull", path);

    b32 result_d = RemoveDirectory(path);
    check(result_d == TRUE, "Couldn't delete folder %s", path);

    return TRUE;
    error:
    return FALSE;
}


internal b32
os_folder_delete_str(String* path) {
    check(path != NULL, "path is NULL");

    b32 result = os_folder_delete_c_str(string_get_c_str(path));
    return result;

    error:
    return FALSE;
}


internal b32
os_folder_exists_c_str(const char* path) {
    check(path != NULL, "path is NULL");

    DWORD result = GetFileAttributesA(path);
    if (result != INVALID_FILE_ATTRIBUTES &&
        (result & FILE_ATTRIBUTE_DIRECTORY) != 0) return TRUE;

    error:
    return FALSE;
}


internal b32
os_folder_exists_str(String* path) {
    check(path != NULL, "path is NULL");

    b32 result = os_folder_exists_c_str(string_get_c_str(path));
    return result;

    error:
    return FALSE;
}
