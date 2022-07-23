#include "common.h"
#include "tests.h"


internal TestStatus
mock_test_fn() {
    return TEST_UNIMPLEMENTED;
}


TestInfo tests[] = {
    { mock_test_fn, "mock_test" },
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