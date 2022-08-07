/* date = August 3rd 2022 8:22 pm */

#ifndef __CONTAINERS_HASH_H__
#define __CONTAINERS_HASH_H__

#include "common.h"
#include "list.h"
#include "utils/memory.h"


typedef struct HashMap HashMap;

typedef u32 (HashFn) (HashMap*, void*);

// NOTE: Tight structures????? meanning that all the data is after the header
// NOTE: like here, the buckets will point directly after the map, but it cannot be resized with the normal function

// NOTE: this uses chaining
// NOTE: The HashMap never takes ownership over the keys and the values
struct HashMap {
    u32 n_buckets;
    u32 length;
    HashFn* hash_fn;
    CompareFn* compare_fn;
    List* buckets; // the buckets are placed directly after the 
};


typedef struct HashPair {
    void* key;
    void* value;
} HashPair;


internal inline u32 hash_u32(HashMap* map, void* p) {
    u32 a = *((int*)p);
    return a % map->n_buckets;
}


internal HashMap* map_create(u32 n_entries,
                             HashFn* hash_fn,
                             CompareFn* compare_fn);

internal void map_destroy(HashMap* map);

internal void map_add(HashMap* map, void* key, void* value);
internal void* map_get(HashMap* map, void* key);
internal void* map_remove(HashMap* map, void* key);


#endif // __CONTAINERS_HASH_H_
