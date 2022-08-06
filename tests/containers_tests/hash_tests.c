#include "common.h"
#include "tests.h"
#include "containers/hash.h"
#include "containers/hash.c"


internal TestStatus
hash_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    HashMap* map = NULL;
    u32 n_entries = 32;
    
    map = map_create(n_entries, hash_u32, compare_u32);
    assert(map != NULL, "map is NULL");
    assert(map->n_buckets == n_entries, "map->n_buckets is %u, not %u",
           map->n_buckets, n_entries);
    assert(map->length == 0, "map->length is %u, not %u",
           map->length, 0);
    assert(map->hash_fn != NULL, "map->hash_fn is %p, not %p",
           map->hash_fn, hash_u32);
    assert(map->compare_fn != NULL, "map->compare_fn is %p, not %p",
           map->compare_fn, compare_u32);
    assert(map->buckets != NULL, "map->buckets is NULL");
    for (u32 i = 0; i < map->n_buckets; ++i) {
        assert(list_is_empty(&(map->buckets[i])) != 0,
               "List %u should be empty", i);
    }
    
    map_destroy(map);
    
    map = map_create(0, hash_u32, compare_u32);
    assert(map == NULL, "should be NULL for n_entris of 0");
    map = map_create(n_entries, NULL, compare_u32);
    assert(map == NULL, "should be NULL for hash_fn of NULL");
    map = map_create(n_entries, hash_u32, NULL);
    assert(map == NULL, "should be NULL for compare_fn of NULL");
    map_destroy(NULL);
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
hash_add_get_remove_test() {
    TestStatus status = TEST_FAILED;
    HashMap* map = NULL;
    void* ptr = NULL;
    u32 n_entries = 32;
    u32 key1 = 0;
    u32 key2 = 54;
    u32 key3 = 124;
    u32 val = 0;
    
    // map create
    map = map_create(n_entries, hash_u32, compare_u32);
    map_add(map, &key1, &key1);
    map_add(map, &key2, &key2);
    map_add(map, &key3, &key3);
    map_add(map, &key3, &key3);
    assert(map->length == 4, "map->length is %u, should be %u",
           map->length, 4);
    
    // map get
    ptr = map_get(map, &key1);
    assert(ptr != NULL, "sould return %p not NULL", &key1);
    val = *((u32*)ptr);
    assert(val == key1, "value for key %u should be %u, not %u",
           key1, key1, val);
    
    ptr = map_get(map, &key2);
    assert(ptr != NULL, "sould return %p not NULL", &key2);
    val = *((u32*)ptr);
    assert(val == key2, "value for key %u should be %u, not %u",
           key2, key2, val);
    
    ptr = map_get(map, &key3);
    assert(ptr != NULL, "sould return %p not NULL", &key3);
    val = *((u32*)ptr);
    assert(val == key3, "value for key %u should be %u, not %u",
           key3, key3, val);
    
    // map remove
    ptr = map_remove(map, &key1);
    assert(ptr != NULL, "should return %p not NULL", &key1);
    val = *((u32*)ptr);
    assert(val == key1, "value for key %u should be %u, not %u",
           key1, key1, val);
    assert(map->length == 3, "length should be %u, not %u", 
           3 , map->length);
    
    ptr = map_remove(map, &key2);
    assert(ptr != NULL, "should return %p not NULL", &key2);
    val = *((u32*)ptr);
    assert(val == key2, "value for key %u should be %u, not %u",
           key2, key2, val);
    assert(map->length == 2, "length should be %u, not %u", 
           2, map->length);
    
    ptr = map_remove(map, &key3);
    assert(ptr != NULL, "should return %p not NULL", &key3);
    val = *((u32*)ptr);
    assert(val == key3, "value for key %u should be %u, not %u",
           key3, key3, val);
    assert(map->length == 1, "length should be %u, not %u", 
           1, map->length);
    
    ptr = map_remove(map, &key3);
    assert(ptr != NULL, "should return %p not NULL", &key3);
    val = *((u32*)ptr);
    assert(val == key3, "value for key %u should be %u, not %u",
           key3, key3, val);
    assert(map->length == 0, "length should be %u, not %u", 
           0, map->length);
    
    ptr = map_remove(map, &key3);
    assert(ptr == NULL, "should return NULL for empty map");
    
    // EDGE cases
    map_add(NULL, &key1, &key1);
    map_add(map, NULL, &key1);
    map_add(map, &key1, NULL);
    
    ptr = map_get(NULL, &key1);
    assert(ptr == NULL, "Should get NULL for NULL map");
    ptr = map_get(map, NULL);
    assert(ptr == NULL, "Should get NULL for NULL key");
    
    ptr = map_remove(NULL, &key1);
    assert(ptr == NULL, "Should get NULL for NULL map");
    ptr = map_remove(map, NULL);
    assert(ptr == NULL, "Should get NULL for NULL key");
    
    
    map_destroy(map);
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
hash_resize_map_test() {
    TestStatus status = TEST_FAILED;
    HashMap* map = map_create(2, hash_u32, compare_u32);
    u32 key1 = 23;
    u32 key2 = 124;
    u32 key3 = 2351;
    void* ptr = NULL;
    u32 val = 0;
    
    map_add(map, &key1, &key1);
    map_add(map, &key2, &key2);
    map_add(map, &key3, &key3);
    
    ptr = map_remove(map, &key3);
    assert(ptr != NULL, "should return %p not NULL", &key3);
    val = *((u32*)ptr);
    assert(val == key3, "value for key %u should be %u, not %u",
           key3, key3, val);
    assert(map->length == 2, "length should be %u, not %u", 
           0, map->length);
    
    ptr = map_remove(map, &key2);
    assert(ptr != NULL, "should return %p not NULL", &key2);
    val = *((u32*)ptr);
    assert(val == key2, "value for key %u should be %u, not %u",
           key2, key2, val);
    assert(map->length == 1, "length should be %u, not %u", 
           0, map->length);
    
    ptr = map_remove(map, &key1);
    assert(ptr != NULL, "should return %p not NULL", &key1);
    val = *((u32*)ptr);
    assert(val == key1, "value for key %u should be %u, not %u",
           key1, key1, val);
    assert(map->length == 0, "length should be %u, not %u", 
           0, map->length);
    
    ptr = map_remove(map, &key3);
    assert(ptr == NULL, "should return NULL for empty map");
    
    assert(map->n_buckets == 4, "should have 4 buckets not %u",
           map->n_buckets);
    
    map_destroy(map);
    
    status = TEST_SUCCESS;
    error:
    return status;
}