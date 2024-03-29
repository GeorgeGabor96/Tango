/* date = July 23rd 2022 11:30 pm */

#ifndef TESTS_H
#define TESTS_H

#include "utils/memory.h"


// TODO: Shouldn't this be a function call? do we need the errno print and set?
#define assert(test, message, ...) if (!(test)) {\
fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " message "\n",\
__FILE__,\
__LINE__,\
errno_text(),\
##__VA_ARGS__);\
errno=0;\
goto error; }

#define FLOAT_PRECISION 0.001
#define float_equal(a, b) ((a) < (b) + FLOAT_PRECISION && (a) > (b) - FLOAT_PRECISION)


typedef enum {
    TEST_SUCCESS = 0,
    TEST_FAILED = 1,
    TEST_UNIMPLEMENTED = 2
} TestStatus;

typedef TestStatus (*TestFunction)();

typedef struct TestInfo {
	TestFunction function;
	char* name;
} TestInfo;


internal inline b32 memory_leak() {
    b32 leak = !memory_is_empty();
	if (leak == TRUE) memory_report();
	return leak;
}

#endif //TESTS_H
