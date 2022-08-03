#include "hash.h"


// HELPER functions
internal inline size_t
get_map_size(u32 n_buckets) {
    return sizeof(Map) + sizeof(List) * n_buckets;
}


internal inline Node*
map_find_node(Map* map, void* key) {
    u32 bucket_i = map->hash_fn(map, key);
    List* bucket = map->buckets[bucket_i];
    Node* iter = bucket->first;
    
    while (iter != NULL) {
        // NOTE: The value in a node is dirrectly after the node
        HashNode* h_node = iter + 1;
        if (map->compare_keys(h_mode->key, key) == 0) {
            break;
        }
        iter = iter->next;
    }
    return iter;
}


internal inline HashMap*
map_resize_check(HashMap* map) {
    // NOTE: if fill factor above 80% we resize
    if ((float)(map->length) / (float)(map->n_buckets) < 0.8) {
        return;
    }
    
    u32 old_length = map->length;
    u32 new_n_buckets = map->n_buckets ** 2;
    size_t map_new_size = get_map_size(new_n_buckets);
    HashMap* new_table = map_create(new_n_buckets, map->hash_fn);
    
    
    map->n_buckets = new_table->n_buckets;
    map->buckets = new_table->buckets;
    free(new_table); // don
    // TODO: need to continue this
}


// Actual functions
internal Map*
map_create(u32 n_buckets, HashFn* hash_fn) {
    check(n_buckets > 0, "n_entries is 0");
    check(hash_fn != NULL, "hash_fn cannot be NULL");
    
    size_t map_size = get_map_size(n_buckets);
    Map* map = (Map*) malloc(map_size);
    check_memory(map);
    
    map->n_buckets = n_buckets;
    map->length = 0;
    map->hash_fn = hash_fn;
    map->buckets = map + 1;
    for (u32 i = 0; i < n_buckets; ++i) {
        list_init(map->buckets[i], sizeof(HashNode));
    }
    
    return map;
    
    error:
    return NULL;
}


internal void
map_destroy(Map* map, ResetFn* reset_fn) {
    check(map != NULL, "map is NULL");
    
    for (u32 i = 0; i < map->n_buckets; ++i) {
        list_reset(map->buckets[i], reset_fn);
    }
    map->n_buckets = 0;
    map->length = 0;
    map->hash_fn = NULL;
    free(map);
    
    error:
    return;
}


internal void
map_add(Map* map, void* key, void* value) {
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    check(element != NULL, "element is NULL");
    
    u32 bucket_i = map->hash_fn(map, key);
    List* bucket = map->buckets[bucket_i];
    HashNode h_node;
    h_node->key = key;
    h_node->value = value;
    list_add_last(bucket, h_node);
    ++(map->length);
    
    map_resize_check(map);
    
    error:
    return;
}


internal HashPair
map_get(Map* map, void* key) {
    HashNode h_node_copy;
    
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    
    Node* node_with_key = map_find_node(map, key);
    if (iter != NULL) {
        HashNode* h_node = iter + 1;
        result.key = h_node->key;
        result.value = h_node->value;
    }
    error:
    return result;
}


internal HashPair
map_remove(Map* map, void* key) {
    HashNode h_node_copy;
    
    check(map != NULL, "map is NULL");
    check(key != NULL, "key is NULL");
    
    Node* node_with_key = map_find_node(map, key);
    if (iter != NULL) {
        HashNode* h_node = iter + 1;
        result.key = h_node->key;
        result.value = h_node->value;
        
        // NOTE: remove the node from the list and free it
        list_unlink_node(bucket, iter);
        free(iter);
        
        --(map->length);
    }
    error:
    return result;
}


