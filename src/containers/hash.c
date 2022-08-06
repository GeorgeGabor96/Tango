#include "hash.h"


// HELPER functions
internal inline Node*
map_find_node(HashMap* map, void* key) {
    u32 bucket_i = map->hash_fn(map, key);
    List bucket = map->buckets[bucket_i];
    Node* iter = bucket.first;
    
    while (iter != NULL) {
        // NOTE: The value in a node is dirrectly after the node
        HashPair* pair = (HashPair*)(iter + 1);
        if (map->compare_fn(pair->key, key) == 0) {
            break;
        }
        iter = iter->next;
    }
    return iter;
}


internal inline void
map_resize_check(HashMap* map) {
    // NOTE: if fill factor above 80% we resize
    if ((float)(map->length) / (float)(map->n_buckets) < 0.8) {
        return;
    }
    
    u32 old_n_buckets = map->n_buckets;
    List* old_buckets = map->buckets; 
    
    map->n_buckets = map->n_buckets * 2;
    map->buckets = (List*)malloc(sizeof(List) * map->n_buckets);
    check_memory(map->buckets);
    
    map->length = 0;
    for (u32 i = 0; i < map->n_buckets; ++i) {
        list_init(&(map->buckets[i]), sizeof(HashPair));
    }
    
    // NOTE: Need to get every node from the old hash map into the new one
    for (u32 i = 0; i < old_n_buckets; ++i) {
        List* list = &(old_buckets[i]);
        
        while (list_is_empty(list) == 0) {
            HashPair pair;
            list_remove_first(list, &pair);
            
            map_add(map, pair.key, pair.value);
        }
        
        list_reset(list, NULL);
    }
    free(old_buckets);
    
    error:
    return;
}


// Actual functions
internal HashMap*
map_create(u32 n_buckets, HashFn* hash_fn, CompareFn* compare_fn) {
    HashMap* map = NULL;
    u32 i = 0;
    
    check(n_buckets > 0, "n_entries is 0");
    check(hash_fn != NULL, "hash_fn cannot be NULL");
    check(compare_fn != NULL, "compare_fn cannot be NULL");
    
    map = (HashMap*)malloc(sizeof(HashMap));
    check_memory(map);
    
    map->buckets = (List*)malloc(sizeof(List) * n_buckets);
    check_memory(map->buckets);
    map->n_buckets = n_buckets;
    map->length = 0;
    map->hash_fn = hash_fn;
    map->compare_fn = compare_fn;
    for (i = 0; i < map->n_buckets; ++i) {
        list_init(&(map->buckets[i]), sizeof(HashPair));
    }
    
    return map;
    
    error:
    if (map != NULL) free(map);
    return NULL;
}


internal void
map_destroy(HashMap* map) {
    u32 i = 0;
    
    check(map != NULL, "map is NULL");
    
    // NOTE: the hash pair si just two pointer that I save, no need to 
    // NOTE: do anything when destroying the lists
    for (u32 i = 0; i < map->n_buckets; ++i) {
        list_reset(&(map->buckets[i]), NULL);
    }
    map->n_buckets = 0;
    map->length = 0;
    map->hash_fn = NULL;
    map->compare_fn = NULL;
    free(map->buckets);
    free(map);
    
    error:
    return;
}


internal void
map_add(HashMap* map, void* key, void* value) {
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    check(value != NULL, "value is NULL");
    
    u32 bucket_i = map->hash_fn(map, key);
    List* bucket = &(map->buckets[bucket_i]);
    HashPair pair;
    pair.key = key;
    pair.value = value;
    list_add_last(bucket, &pair);
    
    ++(map->length);
    
    map_resize_check(map);
    
    error:
    return;
}


internal void*
map_get(HashMap* map, void* key) {
    void* value = NULL;
    Node* node = NULL;
    HashPair* pair = NULL;
    
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    
    u32 bucket_idx = map->hash_fn(map, key);
    List* list = &(map->buckets[bucket_idx]);
    
    node = list->first;
    while (node != NULL) {
        HashPair* pair = (HashPair*)(node + 1);
        if (map->compare_fn(pair->key, key) == 1) {
            value = pair->value;
            break;
        }
        node = node->next;
    }
    return value;
    error:
    return NULL;
}


internal void*
map_remove(HashMap* map, void* key) {
    Node* node = NULL;
    HashPair* pair = NULL;
    void* value = NULL;
    
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    
    u32 bucket_idx = map->hash_fn(map, key);
    List* list = &(map->buckets[bucket_idx]);
    
    node = list->first;
    while (node != NULL) {
        HashPair* pair = (HashPair*)(node + 1);
        if (map->compare_fn(pair->key, key) == 1) {
            value = pair->value;
            list_unlink_node(list, node);
            list_node_destroy(node, sizeof(HashPair));
            --(map->length);
            break;
        }
        node = node->next;
    }
    error:
    return value;
}


