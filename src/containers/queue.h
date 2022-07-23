/* date = July 23rd 2022 11:08 pm */

#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue {
    u32 length;
    u32 head;
    u32 tail;
    u8* data;  // NOTE: this should point directly after the Queue
}

#endif //QUEUE_H
