#include "containers/queue.h"


// HELPER functions
internal inline u8*
queue_el_adr(Queue* queue, u32 idx) {
    return queue->data + idx * queue->el_size;
}


internal inline size_t
queue_data_size(Queue* queue) {
    return queue->el_size * queue->max_length;
}


// ACTUAL functions
internal Queue*
queue_create(u32 length, size_t el_size) {
    check(length != 0, "length is 0");
    check(el_size != 0, "el_size is 0");
    
    Queue* queue = NULL;
    size_t size = queue_size_from_params(length, el_size);
    
    queue = (Queue*)memory_malloc(size, "queue_create");
    check_memory(queue);
    
    queue_init(queue, length, el_size);
    return queue;
    error:
    return NULL;
}


internal void
queue_init(Queue* queue, u32 length, size_t el_size) {
    check(queue != NULL, "NULL queue");
    check(length != 0, "length is 0");
    check(el_size != 0, "el_size is 0");
    
    queue->max_length = length;
    queue->length = 0;
    queue->el_size = el_size;
    queue->tail = 0;
    queue->head = 0;
    queue->data = (u8*)(queue + 1);
    
    // TODO: shouldn't always set to 0??
    memset(queue->data, 0, queue_data_size(queue));
    
    error:
    return;
}


internal void
queue_destroy(Queue* queue, ResetFn* reset_fn) {
    check(queue != NULL, "NULL queue");
    queue_reset(queue, reset_fn);
    memory_free(queue);
    error:
    return;
}


internal void
queue_reset(Queue* queue, ResetFn* reset_fn) {
    check(queue != NULL, "NULL queue");
    if (reset_fn != NULL) {
        while(queue->length != 0) {
            queue_dequeue(queue);
        }
    }
    memset(queue->data, 0, queue_data_size(queue));
    queue->max_length = 0;
    queue->length = 0;
    queue->el_size = 0;
    queue->tail = 0;
    queue->head = 0;
    queue->data = NULL;
    error:
    return;
}


internal void
queue_enqueue(Queue* queue, void* element) {
    // Don't expand the queue if its full
    check(queue != NULL, "NULL queue");
    check(element != NULL, "NULL element");
    check(queue_is_full(queue) == 0, "Queue is FULL");
    
    u8* tail_adr = queue_el_adr(queue, queue->tail);
    memcpy(tail_adr, element, queue->el_size);
    ++(queue->tail);
    ++(queue->length);
    if (queue->tail == queue->max_length) {
        queue->tail = 0;
    }
    
    error:
    return;
}


internal void*
queue_dequeue(Queue* queue) {
    // TODO: This can cause problems in the case the queue is full
    // TODO: if I dequeue it and don't use the pointer and do an enqueue
    // TODO: it can overwrite the data at that adress
    check(queue != NULL, "NULL queue");
    
    u8* head_adr = queue_el_adr(queue, queue->head);
    ++(queue->head);
    --(queue->length);
    if (queue->head == queue->max_length) {
        queue->head = 0;
    }
    return head_adr;
    
    error:
    return NULL;
}


internal void
queue_dequeue_copy(Queue* queue, void* element) {
    // NOTE: dequeue and copy the element into a placeholder to avoid
    // NOTE: overwrite of data in pointer
    check(queue != NULL, "NULL queue");
    check(element != NULL, "NULL queue");
    
    u8* el_adr = queue_dequeue(queue);
    check(el_adr != NULL, "el_adr is NULL");
    memcpy(element, el_adr, queue->el_size);
    
    error:
    return;
}


internal void
queue_show(Queue* queue, ShowFn* show_fn) {
    check(queue != NULL, "NULL queue");
    check(show_fn != NULL, "NULL show_fn");
    
    if (queue_is_empty(queue) != 0)
        return;
    
    u32 head_i = queue->head;
    u32 tail_i = queue->tail;
    
    // NOTE: Show from head to tail
    // NOTE: need the do while in order to modify the head before check in case head
    // NOTE: and tail are equal
    do {
        u8* el_adr = queue_el_adr(queue, head_i);
        show_fn(el_adr);
        printf(" ");
        ++head_i;
        if (head_i == queue->max_length) head_i = 0;
    } while (head_i != tail_i); 
    printf("\n");
    
    error:
    return;
}


internal u32
queue_is_empty(Queue* queue) {
    check(queue != NULL, "NULL queue");
    return queue->length == 0;
    error:
    return 0;
}


internal u32
queue_is_full(Queue* queue) {
    check(queue != NULL, "NULL queue");
    return queue->length == queue->max_length;
    error:
    return 0;
}



internal size_t
queue_size_from_params(u32 length, size_t el_size) {
    return sizeof(Queue) + length * el_size;
}


internal size_t
queue_size(Queue* queue) {
    check(queue != NULL, "NULL queue");
    size_t size = queue_size_from_params(queue->max_length, queue->el_size);
    return size;
    error:
    return 0;
}