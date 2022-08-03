/* date = August 3rd 2022 8:22 pm */

#ifndef __CONTAINERS_HASH_H__
#define __CONTAINERS_HASH_H__

#include "common.h"
#include "list.h"

typedef void* HashMap;

typedef u32 (HashFn) (HashMap*, void*);


// NOTE: this uses chaining
typedef struct HashMap {
    u32 n_buckets;
    u32 length;
    HashFn* hash_fn;
    List* buckets; // the buckets are placed directly after the structure
} HashMap;


typedef struct HashPair {
    void* key;
    void* value;
} HashPair;



internal Map* map_create(u32 n_entries, HashFn* hash_fn);
internal void map_destroy(Map* map, ResetFn* reset_fn);
internal void map_add(Map* map, void* key, void* value);
internal HashPair map_get(Map* map, void* key);
internal HashPair map_remove(Map* map, void* key);


#endif // __CONTAINERS_HASH_H_
