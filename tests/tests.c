#include "common.h"
#include "tests.h"

#include "containers/array.h"
#include "containers/array.c"

#include "utils_tests/os_tests.c"
#include "utils_tests/memory_tests.c"
#include "containers_tests/string_tests.c"
#include "containers_tests/memory_arena_tests.c"
#include "simulator_tests/synapse_tests.c"
#include "simulator_tests/neuron_tests.c"


TestInfo tests[] = {
    // OS test
    { os_directory_test, "os_directory_test" },
    // Memory tests
    { memory_general_test, "memory_management_general_test" },
    { memory_report_test, "memory_report_test" },
    // Memory Arena tests
    { memory_arena_create_destroy_test, "memory_arena_create_destroy_test" }, 
    { memory_arena_push_test, "memory_arena_push" },
    { memory_arena_alignment_test, "memory_arena_alignment_test" }, 
    // String tests
    { string_create_destroy_test, "string_create_destroy_test" },
    { string_path_join_test, "string_path_join_test" },
    { string_to_c_str_test, "string_to_c_str_test" },
    { string_equal_test, "string_equal_test" },
    { string_equal_c_str_test, "string_equal_c_str_test" },
    /*******************
    * SIMULATOR TESTS
    *******************/
    // Synapse tests
    { synapse_create_destroy_test, "synapse_create_destroy_test" },
    { synapse_behaviour_test, "synapse_behaviour_test" },
    // Neuron tests
    { neuron_create_destroy_test, "neuron_create_destroy_test" },
    { neuron_step_test, "neuron_step_test" },
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