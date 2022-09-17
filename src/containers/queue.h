/* date = July 23rd 2022 11:08 pm */

#ifndef __CONTAINERS_QUEUE_H__
#define __CONTAINERS_QUEUE_H__

#include "common.h"
#include "utils/memory.h"


typedef struct Queue {
    u32 head;   // remove
    u32 tail;   // add
    u32 max_length;
    u32 length;
    size_t el_size;
    u8* data;  // NOTE: this should point directly after the Queue
} Queue;


internal Queue* queue_create(u32 length, size_t el_size);
internal void queue_init(Queue* queue, u32 length, size_t el_size);

internal void queue_destory(Queue* queue, ResetFn* reset_fn);
internal void queue_reset(Queue* queue, ResetFn* reset_fn);

internal void queue_enqueue(Queue* queue, void* element);
internal void* queue_head(Queue* queue);
internal void* queue_dequeue(Queue* queue);

internal void queue_show(Queue* queue, ShowFn* show_fn);
internal bool queue_is_empty(Queue* queue);
internal bool queue_is_full(Queue* queue);

internal size_t queue_size(Queue* queue);
internal size_t queue_size_from_params(u32 length, size_t el_size);

#endif // __CONTAINERSQUEUE_H__
