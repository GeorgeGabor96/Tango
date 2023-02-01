#include <Windows.h>


typedef struct ThreadPoolQueue {
	void* tasks[MAX_N_TASKS];
    u32 n_tasks;
    u32 task_i;
    b32 empty;
} ThreadPoolQueue;  // NOTE: Not actually a queue:)


struct _ThreadPool {
	HANDLE* threads;
    u32 n_threads;
    u32 n_working_threads;

	ThreadPoolQueue* queue;
    b32 stop;

	THREAD_POOL_TASK_EXECUTE execute;

	CRITICAL_SECTION lock;
	CONDITION_VARIABLE start_work_signal;
    CONDITION_VARIABLE end_work_signal;
} _ThreadPool;


inline internal b32
process_task(ThreadPool* pool) {
    u32 task_i = 0;
    u32 n_tasks = 0;
    b32 empty = FALSE;
    void* task = NULL;

	EnterCriticalSection(&(pool->lock));
    task_i = pool->queue->task_i;
	n_tasks = pool->queue->n_tasks;
	empty = pool->queue->empty;

	if (empty == TRUE) {
		LeaveCriticalSection(&(pool->lock));
		return FALSE;
	}

    // NOTE: the queue should be empty, set it and return
	if (task_i >= n_tasks) {
		pool->queue->empty = TRUE;
		LeaveCriticalSection(&(pool->lock));
		return FALSE;
	}

	++(pool->queue->task_i);
    // NOTE: increment the number of thread that work, so that we wait for them to finish
	++(pool->n_working_threads);
    LeaveCriticalSection(&(pool->lock));

	task = pool->queue->tasks[task_i];
    pool->execute(task);

    // NOTE: decrement the number of threads that work and signal that the work may be finish
    EnterCriticalSection(&(pool->lock));
    --(pool->n_working_threads);
    LeaveCriticalSection(&(pool->lock));
    // NOTE: this may be the last thread that was working, signal this so that we can continue
    WakeConditionVariable(&(pool->end_work_signal));

	return TRUE;
}


inline internal DWORD WINAPI
pool_execute(LPVOID lpParameter) {
	ThreadPool* pool = (ThreadPool*)lpParameter;

	while (TRUE) {
		EnterCriticalSection(&(pool->lock));
		// NOTE: wait for the start of processing to signalled
        while (pool->queue->empty == TRUE && pool->stop == FALSE) {
			SleepConditionVariableCS(&(pool->start_work_signal), &(pool->lock), INFINITE);
		}
		if (pool->stop == TRUE) {
			LeaveCriticalSection(&(pool->lock));
			break;
		}

		LeaveCriticalSection(&(pool->lock));

		while (process_task(pool) == TRUE) {}
	}
	return 0;
}


internal ThreadPool*
thread_pool_create(u32 n_threads,
                   THREAD_POOL_TASK_EXECUTE execute,
                   Memory* memory) {
    check(execute != NULL, "execute is NULL");
    check(memory != NULL, "memory is NULL");

    u32 i = 0;
	DWORD thread_id = 0;
	ThreadPoolQueue* queue = NULL;

	ThreadPool* pool = (ThreadPool*)memory_push(memory, sizeof(*pool));
	if (pool == NULL) return NULL;

	InitializeCriticalSection(&(pool->lock));
	InitializeConditionVariable(&(pool->start_work_signal));
    InitializeConditionVariable(&(pool->end_work_signal));

	// NOTE: Like a memory fence, to make sure the instructions are not rearange and the threads start before everything is initialized
	EnterCriticalSection(&(pool->lock));

	queue = (ThreadPoolQueue*)memory_push(memory, sizeof(ThreadPoolQueue));
	if (queue == NULL) {
		LeaveCriticalSection(&(pool->lock));
		return NULL;
	}
	pool->n_working_threads = 0;
    queue->n_tasks = 0;
	queue->task_i = 0;
	queue->empty = TRUE;
	pool->queue = queue;

	pool->stop = FALSE;
	pool->execute = execute;
	if (n_threads == 0) {
        pool->threads = NULL;
    } else {
        pool->threads = (HANDLE*)memory_push(memory, sizeof(HANDLE) * n_threads);
        if (pool->threads == NULL) {
            LeaveCriticalSection(&(pool->lock));
            return NULL;
        }
    }

	LeaveCriticalSection(&(pool->lock));

	for (i = 0; i < n_threads; ++i) {
		pool->threads[i] = CreateThread(0, 0, pool_execute, pool, 0, &thread_id);
		if (pool->threads[i] == NULL) return NULL;
	}
	pool->n_threads = i;
	return pool;

    error:
    return NULL;
}


internal void
thread_pool_reset(ThreadPool* pool) {
	check(pool != NULL, "pool is NULL");

    EnterCriticalSection(&(pool->lock));

    pool->n_working_threads = 0;
	pool->queue->n_tasks = 0;
	pool->queue->task_i = 0;
    pool->queue->empty = TRUE;

	LeaveCriticalSection(&(pool->lock));

    error:
    return;
}


internal void
thread_pool_add_task(ThreadPool* pool, void* task) {
	check(pool != NULL, "pool is NULL");
    check(task != NULL, "task is NULL");

    EnterCriticalSection(&(pool->lock));

	if (pool->queue->n_tasks == MAX_N_TASKS) {
		log_warning("Increase queue size");
		LeaveCriticalSection(&(pool->lock));
		return;
	}

	pool->queue->tasks[pool->queue->n_tasks] = task;
	++(pool->queue->n_tasks);

	LeaveCriticalSection(&(pool->lock));

    error:
    return;
}


internal void
thread_pool_execute_tasks(ThreadPool* pool) {
	check(pool != NULL, "pool is NULL");

    // NOTE: queue is not empty signal the workers to start
    EnterCriticalSection(&(pool->lock));
	pool->queue->empty = FALSE;
	LeaveCriticalSection(&(pool->lock));
	WakeAllConditionVariable(&(pool->start_work_signal));

    // NOTE: the main thread is also working
    while (process_task(pool) == TRUE) {}

    // NOTE: make sure all the threads finished their work before continuing
    EnterCriticalSection(&(pool->lock));
    while (pool->n_working_threads > 0)
        SleepConditionVariableCS(&(pool->end_work_signal), &(pool->lock), INFINITE);
    LeaveCriticalSection(&(pool->lock));

    error:
    return;
}


internal void
thread_pool_stop(ThreadPool* pool) {
	check(pool != NULL, "pool is NULL");

    // NOTE: signal the workers that they should finish the execution and wait for them
    EnterCriticalSection(&(pool->lock));
	pool->stop = TRUE;
	LeaveCriticalSection(&(pool->lock));
	WakeAllConditionVariable(&(pool->start_work_signal));

	for (u32 i = 0; i < pool->n_threads; ++i) {
		WaitForSingleObject(pool->threads[i], INFINITE);
		CloseHandle(pool->threads[i]);
	}

    error:
    return;
}
