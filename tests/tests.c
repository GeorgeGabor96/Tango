#include "common.h"
#include "tests.h"

#include "containers_tests/array_tests.c"
#include "containers_tests/queue_tests.c"
#include "containers_tests/list_tests.c"


TestInfo tests[] = {
    // Array tests
    { array_create_destroy_test, "array_create_destroy_test" },
    { array_set_get_test, "array_set_get_test" },
    { array_swap_test, "array_swap_test" },
    { array_show_test, "array_show_test" },
    { array_size_test, "array_size_test" },
    // Queue tests
    { queue_create_destroy_test, "queue_create_destroy_test" },
    { queue_enqueue_dequeue_test, "queue_enqueue_dequeue_test" },
    { queue_show_test, "queue_show_test" },
    { queue_size_test, "queue_size_test" },
    // List tests
    { list_create_destroy_test, "list_create_destroy_test" },
    { list_add_remove_test, "list_add_remove_test" },
    { list_show_test, "list_show_test" },
};


int main() {
    u32 n_tests_runned = 0;
    u32 n_tests_success = 0;
    u32 n_tests_failed = 0;
    u32 n_tests_unimplemented = 0;
    u32 i = 0;
    u32 n_tests = sizeof(tests) / sizeof(TestInfo);
    TestStatus status = TEST_FAILED;
    
    for (i = 0; i < n_tests; ++i) {
        log_info("Running test %s", tests[i].name);
        ++n_tests_runned;
        
        status = tests[i].function();
        if (status == TEST_SUCCESS) {
            log_info("Test %s SUCCESS\n", tests[i].name);
            ++n_tests_success;
        }
        else if (status == TEST_FAILED) {
            log_error("Test %s FAIL\n", tests[i].name);
            ++n_tests_failed;
            break;
        }
        else if (status == TEST_UNIMPLEMENTED) {
            log_warning("Test %s UNIMPLEMENTED\n", tests[i].name);
            ++n_tests_unimplemented;
        }
    }
    
    log_info("Number of RUNNED tests: %d\n", n_tests_runned);
    log_info("Number of SUCCESSFULL tests: %d\n", n_tests_success);
    log_info("Number of FAILED tests: %d\n", n_tests_failed);
    log_info("Number of UNIMPLEMENTED tests: %d\n", n_tests_unimplemented);
    return 0;
}