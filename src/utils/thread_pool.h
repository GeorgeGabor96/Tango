#ifndef __UTILS_THREAD_POOL_H
#define __UTILS_THREAD_POOL_H


#define MAX_N_TASKS 256


typedef struct _ThreadPool ThreadPool;

typedef void (*THREAD_POOL_TASK_EXECUTE) (void*);


internal ThreadPool* thread_pool_create(u32 n_threads,
                                        THREAD_POOL_TASK_EXECUTE execute,
                                        Memory* memory);

// CALL before adding the set of tasks
internal void thread_pool_reset(ThreadPool* pool);

internal void thread_pool_add_task(ThreadPool* pool, void* task);

// AFter all tasks are in the pool
internal void thread_pool_execute_tasks(ThreadPool* pool);			

internal void thread_pool_stop(ThreadPool* pool);


#endif // __UTILS_THREAD_POOL_H__
