/* date = November 26th 2022 4:05 pm */

#ifndef THREAD_H
#define THREAD_H


#include "common.h"


typedef struct ThreadWorkOrder {
    void* layer;
    u32 neuron_idx_start;
    u32 neuron_idx_end;
    u32 time;
} ThreadWorkOrder;


typedef struct ThreadPool {
    u32 n_orders;
    ThreadWorkOrder* orders;
    
    volatile u32 c_order_idx;
} ThreadPool;


typedef u32 (*THREAD_FUNCTION)(void* parameter);


#endif //THREAD_H
