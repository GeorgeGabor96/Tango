#include "common.h"
#include "tests.h"
#include "containers/queue.h"
#include "containers/queue.c"


internal TestStatus
queue_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    Queue* queue = NULL;
    
    queue = queue_create(10, sizeof(i64));
    
    assert(queue != NULL, "queue is NULL");
    assert(queue->max_length == 10, "queue->max_length is %u it should be 10", queue->max_length);
    assert(queue->length == 0, "queue->length is %u it should be 0", queue->length);
    assert(queue->el_size == sizeof(i64), "queue->el_size si %zu it should be %zu", queue->el_size, sizeof(i64));
    assert(queue->head == 0, "queue->head is %u it should be 0", queue->head);
    assert(queue->tail == 0, "queue->tail is %u it should be 0", queue->tail);
    assert((void*)(queue->data) == (void*)(queue + 1), "queue->data is %p, it should be %p, dirrectly after the structure", queue->data, queue + 1);
    
    queue_destroy(queue, NULL);
    
    queue = queue_create(0, sizeof(i64));
    assert(queue == NULL, "queue should be NULL for length of 0");
    
    queue = queue_create(10, 0);
    assert(queue == NULL, "queue should be NULL for el_size of 0");
    
    queue_destroy(NULL, NULL);
    assert(memory_leak() == 0, "Memory_leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
queue_enqueue_dequeue_test() {
    TestStatus status = TEST_FAILED;
    Queue* queue = NULL;
    u32 val0 = 45;
    u32 val1 = 7891;
    u32 val2 = 98123;
    u32 val3 = 9182312;
    u32 val = 0;
    void* val_p = NULL;
    
    queue = queue_create(3, sizeof(u32));
    
    queue_enqueue(queue, &val0);
    queue_enqueue(queue, &val1);
    queue_enqueue(queue, &val2);
    assert(queue_is_full(queue) != 0, "Queue should be full");
    
    val = *(u32*)queue_dequeue(queue);
    assert(val == val0, "val should be %u, but its %u", val0, val);
    val = *(u32*)queue_dequeue(queue);
    assert(val == val1, "val should be %u, but its %u", val1, val);
    val = *(u32*)queue_dequeue(queue);
    assert(val == val2, "val should be %u, but its %u", val2, val);
    assert(queue_is_empty(queue) != 0, "Queue should be empty");
    
    // NOTE: test that the indexes are circular
    queue_enqueue(queue, &val0);
    queue_enqueue(queue, &val1);
    queue_enqueue(queue, &val2);
    queue_dequeue(queue); // val0
    queue_enqueue(queue, &val3);
    queue_dequeue(queue); // val1
    queue_dequeue(queue); // val2
    
    val = *(u32*)queue_dequeue(queue);
    assert(val == val3, "val should be %u, but its %u", val3, val);
    
    // NOTE: test values are not overwritten
    queue_enqueue(queue, &val0);
    queue_enqueue(queue, &val1);
    queue_enqueue(queue, &val2);
    queue_enqueue(queue, &val3);
    
    val = *(u32*)queue_dequeue(queue);
    assert(val == val0, "val should be %u, but its %u", val0, val);
    val = *(u32*)queue_dequeue(queue);
    assert(val == val1, "val should be %u, but its %u", val1, val);
    val = *(u32*)queue_dequeue(queue);
    assert(val == val2, "val should be %u, but its %u", val2, val);
    assert(queue_is_empty(queue) != 0, "Queue should be empty");
    
    // TODO: maybe test that case that you dequeue, get the pointer and then dequeue more
    // TODO: and enqueue until you write over the pointer you have
    // TODO: This is a special case not sure if we need a test, I mean it will happen
    // TODO: and we have that queue_dequeue_copy for that
    // NOTE: Test dirrectly that
    queue_enqueue(queue, &val0);
    queue_dequeue_copy(queue, &val);
    assert(val == val0, "val should be %u, its %u", val0, val);
    
    // Edge cases
    assert(queue_is_empty(queue) != 0, "Queue should be empty");
    queue_enqueue(NULL, &val);
    assert(queue_is_empty(queue) != 0, "Queue should be empty");
    queue_enqueue(queue, NULL);
    assert(queue_is_empty(queue) != 0, "Queue should be empty");
    
    queue_enqueue(queue, &val0);
    queue_enqueue(queue, &val1);
    queue_enqueue(queue, &val2);
    assert(queue_is_full(queue) != 0, "queue should be full");
    queue_dequeue(NULL);
    assert(queue_is_full(queue) != 0, "queue should be full");
    queue_dequeue_copy(NULL, &val);
    assert(queue_is_full(queue) != 0, "queue should be full");
    queue_dequeue_copy(queue, NULL);
    assert(queue_is_full(queue) != 0, "queue should be full");
    queue_dequeue(queue);
    queue_dequeue(queue);
    queue_dequeue(queue);
    
    queue_destroy(queue, NULL);
    assert(memory_leak() == 0, "Memory_leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
queue_show_test() {
    TestStatus status = TEST_FAILED;
    Queue* queue = queue_create(10, sizeof(u32));
    u32 val0 = 1;
    u32 val1 = 2;
    u32 val2 = 3;
    u32 val3 = 4;
    u32 val4 = 5;
    
    queue_enqueue(queue, &val0);
    queue_enqueue(queue, &val1);
    queue_enqueue(queue, &val2);
    queue_enqueue(queue, &val3);
    queue_enqueue(queue, &val4);
    queue_show(queue, u32_show);
    
    // edge cases
    queue_show(queue, NULL);
    queue_show(NULL, u32_show);
    
    queue_destroy(queue, NULL);
    assert(memory_leak() == 0, "Memory_leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
queue_size_test() {
    TestStatus status = TEST_FAILED;
    u32 length = 10;
    size_t el_size = 12;
    Queue* queue = queue_create(length, el_size);
    size_t actual_size = sizeof(Queue) + length * el_size;
    
    size_t size_of_queue = queue_size(queue);
    assert(size_of_queue == actual_size, "size_of_queue should be %zu, it is %zu",
           size_of_queue, actual_size);
    size_of_queue = queue_size(NULL);
    assert(size_of_queue == 0, "size_of array should be 0 for NULL queue not %zu",
           size_of_queue);
    
    queue_destroy(queue, NULL);
    assert(memory_leak() == 0, "Memory_leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}
