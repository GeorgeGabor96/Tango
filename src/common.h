#ifndef __COMMON__
#define __COMMON__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef size_t sz;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;

#define internal static


// FUNCTION definitions
typedef void (ResetFn) (void*);
typedef void (ShowFn) (void*);
typedef u32 (CompareFn) (void*, void*);

u32 compare_u32(void* ap, void* bp) {
    u32 a = *((int*)ap);
    u32 b = *((int*)bp);
    if (a == b) return 1;
    return 0;
}



// Plotting functions
internal void
i32_show(void* data) {
    i32 value = *((i32*)data);
    printf("%d", value);
}

internal void
u32_show(void* data) {
    u32 value = *((u32*)data);
    printf("%u", value);
}


typedef enum { SUCCESS = 0, FAIL = 1 } Status;

// logging
#define errno_text() (errno == 0 ? "None" : strerror(errno))

#define log_error(M, ...) fprintf(stderr,\
"[ERROR] (%s:%d: errno: %s) " M "\n",\
__FILE__,\
__LINE__,\
errno_text(),\
##__VA_ARGS__)

#define log_warning(M, ...) fprintf(stderr,\
"[WARNING] (%s:%d: errno: %s) " M "\n",\
__FILE__,\
__LINE__,\
errno_text(),\
##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr,\
"[INFO] (%s:%d) " M "\n",\
__FILE__,\
__LINE__,\
##__VA_ARGS__)

#ifndef CHECKS

#define debug(M, ...)
#define check(A, M, ...)
#define check_memory(A)
#define if_check(C, A, M, ...)

#else

#define debug(M, ...) fprintf(stderr,\
"[DEBUG] %s:%d: " M "\n",\
__FILE__,\
__LINE__,\
##__VA_ARGS__)

#define check(A, M, ...) if (!(A)) {\
log_error(M, ##__VA_ARGS__);\
errno=0;\
goto error; }

#define check_memory(A) check((A), "Out of Memory.")

#define if_check(C, A, M, ...) if(C) { check(A, M, ##__VA_ARGS__) }

#endif // CHECKS

#endif // __COMMON__
