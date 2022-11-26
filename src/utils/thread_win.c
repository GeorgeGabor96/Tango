// TODO:
#include "windows.h"


internal u32
thread_lock_add_return_orig_value_u32(volatile u32* value, u32 append) {
    u32 result = (LONG)InterlockedExchangeAdd((volatile LONG*)value, 
                                              (LONG)append);
    return result;
}


internal void
create_worker_thread(THREAD_FUNCTION function, void* parameter) {
    
    HANDLE handle = CreateThread(0, 0,
                                 (LPTHREAD_START_ROUTINE)function,
                                 parameter,
                                 0, NULL);
    CloseHandle(handle);
}