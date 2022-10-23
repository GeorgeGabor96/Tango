#include "tests.h"
#include "utils/os.c"


internal TestStatus
os_directory_test() {
    TestStatus status = TEST_FAILED;
    
    char* folder = "test_folder";
    bool result = os_folder_create_c_str(folder);
    assert(result == TRUE, "couldn't create directory %s", folder);
    
    result = os_folder_exists_c_str(folder);
    assert(result == TRUE, "couldn't find directory %s", folder);
    
    result = os_folder_delete_c_str(folder);
    assert(result == TRUE, "couldn't delete directory %s", folder);
    
    result = os_folder_exists_c_str("folder");
    assert(result == FALSE, "shouldn't find directory %s after deletion", folder);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    status = TEST_SUCCESS;
    
    error:
    return status;
}