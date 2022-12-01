/* date = December 1st 2022 2:56 pm */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H


#include "common.h"
#include "containers/memory_arena.h"


#define MAX_N_TASKS 64


typedef struct _ThreadPool ThreadPool;

typedef void (*THREAD_POOL_TASK_EXECUTE) (void*);


internal ThreadPool* thread_pool_create(u32 n_threads,
                                        THREAD_POOL_TASK_EXECUTE execute,
                                        MemoryArena* arena);

// CALL before adding the set of tasks
internal void thread_pool_reset(ThreadPool* pool);

internal void thread_pool_add_task(ThreadPool* pool, void* task);

// AFter all tasks are in the pool
internal void thread_pool_execute_tasks(ThreadPool* pool);			

internal void thread_pool_stop(ThreadPool* pool);


#endif //THREAD_POOL_H
