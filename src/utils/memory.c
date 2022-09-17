#include "utils/memory.h"


/********************************************************************************
* This will keep nodes in Hash Table, where the key is the memory adress of the 
* allocation. Every node has info about one memory allocation and when memory
* is freed a node is removed from the Hash Table
********************************************************************************/

/********************************************************************************
* General information about a memory allocation
* Will be stored in lists
********************************************************************************/
typedef struct MemoryNode {
	void* ptr;
	char* desc;
	size_t size;
	struct MemoryNode* next;
	struct MemoryNode* prev;
} MemoryNode;


internal inline MemoryNode*
memory_create_node(void* ptr, size_t size, char* desc) {
	MemoryNode* node = (MemoryNode*)malloc(sizeof(MemoryNode));
	if (node == NULL) return node;
	node->ptr = ptr;
	node->desc = desc;
	node->size = size;
	node->next = NULL;
	node->prev = NULL;
	return node;
}


/********************************************************************************
* List of Nodes, are used as buckets for the hash table
********************************************************************************/
typedef struct MemoryList {
	MemoryNode* first;
	MemoryNode* last;
} MemoryList;


internal inline void
memory_list_add(MemoryList* list, MemoryNode* node) {
	if (list->first == NULL) {
		list->first = node;
		list->last = node;
	}
	else {
		list->last->next = node;
		node->prev = list->last;
		list->last = node;
	}
}


internal inline MemoryNode*
memory_list_remove_first(MemoryList* list) {
	MemoryNode* node = NULL;
	if (list->first != NULL) {
		node = list->first;
		if (node == list->last) {
			list->first = NULL;
			list->last = NULL;
		}
		else list->first = node->next;
	}
	return node;
}


internal inline MemoryNode*
memory_list_find(MemoryList* list, void* ptr) {
	MemoryNode* iter = list->first;
	while (iter != NULL && iter->ptr != ptr) iter = iter->next;
	return iter;
}


internal inline void
memory_list_remove(MemoryList* list, MemoryNode* node) {
	if (node == list->first && node == list->last) {
		list->first = NULL;
		list->last = NULL;
	}
	else if (node == list->first) {
		node->next->prev = NULL;
		list->first = node->next;
	}
	else if (node == list->last) {
		node->prev->next = NULL;
		list->last = node->prev;
	}
	else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
    node->prev = NULL;
    node->next = NULL;
}


/********************************************************************************
* Hash table with linked lists
* It grows when the length / n_entries >= 0.8
********************************************************************************/
typedef struct MemoryHashTable {
	MemoryList* entries;
	size_t n_entries;
	size_t length;
} MemoryHashTable;
#define MEMORY_TABLE_INITIAL_LENGTH 997


internal inline MemoryHashTable* meory_hash_table_create(size_t n_entries);
internal inline void memory_hash_table_resize(MemoryHashTable* table, size_t new_size);
internal inline void memory_hash_table_add(MemoryHashTable* table, MemoryNode* node);
internal inline MemoryNode* memory_hash_table_remove(MemoryHashTable* table, void* key);


internal inline u32
memory_is_prime(size_t n) {
	size_t i = 0;
	for (i = 2; i < n / 2; ++i) {
		if (n % i == 0) return 0;
	}
	return 1;
}


internal inline size_t
memory_next_prime(size_t n) {
	if (memory_is_prime(n) == 1) return n;
	size_t radius = 1;
	size_t number = 0;
	while (1) {
		number = n + radius;
		if (memory_is_prime(number) == 1) return number;
		number = n - radius;
		if (memory_is_prime(number) == 1) return number;
		radius++;
	}
	return n;
}


internal inline size_t
memory_hash_f(MemoryHashTable* table, void* address_value) {
	return (size_t)address_value % table->n_entries;
}


internal inline MemoryHashTable*
memory_hash_table_create(size_t n_entries) {
	MemoryHashTable* table = (MemoryHashTable*)malloc(sizeof(MemoryHashTable));
	check_memory(table);
    
	table->entries = (MemoryList*)calloc(n_entries, sizeof(MemoryList));
	check_memory(table->entries);
    
	table->n_entries= n_entries;
	table->length = 0;
    
	return table;
    error:
    if (table != NULL) {
		free(table);
	}
	return NULL;
}


internal inline void
memory_hash_table_resize(MemoryHashTable* table, size_t new_size) {
	size_t i = 0;
	MemoryList* list = NULL;
	MemoryNode* node = NULL;
	size_t hash = 0;
	MemoryHashTable* new_table = memory_hash_table_create(new_size);
    
	// move the nodes from @table to @new_table
	for (i = 0; i < table->n_entries; ++i) {
		list = &(table->entries[i]);
		
		// take all elements from the list
		while ((node = memory_list_remove_first(list)) != NULL) {
			node->next = NULL;
			node->prev = NULL;
			memory_hash_table_add(new_table, node);
		}
	}
    
	// overwrite table
	free(table->entries);	// table->entries contains only empty lists
	table->entries = new_table->entries;
	table->length = new_table->length;
	table->n_entries = new_table->n_entries;
	free(new_table); // no need for this anymore
}


internal inline void
memory_hash_table_add(MemoryHashTable* table, MemoryNode* node) {
	// resize when 80% is filled (nr_elements / nr_entries)
	if ((float)table->length / (float)table->n_entries >= 0.8f) {
		memory_hash_table_resize(table, memory_next_prime(table->n_entries * 2));
	}
	size_t idx = memory_hash_f(table, node->ptr);
	memory_list_add(&(table->entries[idx]), node);
	(table->length)++;
}


internal inline MemoryNode*
memory_hash_table_remove(MemoryHashTable* table, void* key) {
	size_t hash = memory_hash_f(table, key);
	MemoryList* entry = &(table->entries[hash]);
	MemoryNode* node = memory_list_find(entry, key);
	check(node != NULL, "NO node with key %p", key);
	memory_list_remove(entry, node);
	(table->length)--;
	return node;
    error:
    return NULL;
}


/********************************************************************************
* Keep a global Hash Table, we only need one for the whose process
* This will be freed automatically at the end of the program
********************************************************************************/
MemoryHashTable* memory_table = NULL;


/********************************************************************************
* Module Functionality
********************************************************************************/
#ifdef MEMORY_MANAGE
internal void*
memory_malloc(size_t size, char* desc) {
    if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
    
	void* ptr = malloc(size);
	check_memory(ptr);
	
    MemoryNode* node = memory_create_node(ptr, size, desc);
	memory_hash_table_add(memory_table, node);
	return ptr;
    error:
    return NULL;
}


internal void*
memory_calloc(size_t nitems, size_t size, char* desc) {
    if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
	void* ptr = calloc(nitems, size);
	check_memory(ptr);
    
	MemoryNode* node = memory_create_node(ptr, nitems * size, desc);
	memory_hash_table_add(memory_table, node);
	return ptr;
    error:
    return NULL;
}


internal void*
memory_realloc(void* ptr, size_t size, char* desc) {
    if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
    
	// remove node because its key may change
	MemoryNode* node = memory_hash_table_remove(memory_table, ptr);
	check(node != NULL, "%p pointer does not have a node", ptr);
    
    // try to realloc
	void* n_ptr = realloc(ptr, size);
	if (n_ptr == NULL) {
        // NOTE: if failed add the node again
        memory_hash_table_add(memory_table, node);
        n_ptr = ptr;
    } else {
        // update node state
        node->ptr = n_ptr;
        node->size = size;
        node->desc = desc;
        memory_hash_table_add(memory_table, node);
	}
    return n_ptr;
    error:
    // NOTE: the node its not present cannot realloc
    return NULL;
}


internal void
memory_free(void* ptr) {
	if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
    
    // find node and remove it from list
	MemoryNode* node = memory_hash_table_remove(memory_table, ptr);
	check(node != NULL, "Node should exist for %p", ptr);
    
	free(node->ptr);
	free(node);
    error:
    return;
}

#endif

internal size_t
memory_get_n_blocks() {
	if (memory_table == NULL) {
		memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
	}
	size_t i = 0;
	size_t n_blocks = 0;
	MemoryList* list = NULL;
	MemoryNode* node = NULL;
    
	for (i = 0; i < memory_table->n_entries; ++i) {
		list = &(memory_table->entries[i]);
		node = list->first;
        
		while (node != NULL) {
			n_blocks++;
			node = node->next;
		}
	}
	return n_blocks;
}


internal size_t
memory_get_size() {
	if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
	size_t i = 0;
	size_t mem_size = 0;
	MemoryList* list = NULL;
	MemoryNode* node = NULL;
    
	for (i = 0; i < memory_table->n_entries; ++i) {
		list = &(memory_table->entries[i]);
		node = list->first;
        
		while (node != NULL) {
			mem_size += node->size;
			node = node->next;
		}
	}
	return mem_size;
}


internal bool
memory_is_empty() {
	if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
	if (memory_table->length == 0) return TRUE;
	return FALSE;
}


internal void
memory_report() {
	if (memory_table == NULL) 
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
	size_t i = 0;
	size_t mem_size = 0;
	size_t n_nodes = 0;
	MemoryList* list = NULL;
	MemoryNode* node = NULL;
    
	log_info("UNFREED MEMORY");
	for (i = 0; i < memory_table->n_entries; ++i) {
		list = &(memory_table->entries[i]);
		node = list->first;
        
		while (node != NULL) {
			printf("Node %llu - desc: %s ptr: %p size: %llu BYTES\n", n_nodes, node->desc, node->ptr, node->size);
			n_nodes++;
			mem_size += node->size;
			node = node->next;
		}
	}
    
	check(n_nodes == memory_table->length, "@n_nodes != @memory_table->length");
	log_info("Summary %llu nodes, %llu BYTES not freed\n", n_nodes, mem_size);
    error:
    return;
}


internal void
memory_show_inner_state(u32 show_entries, u32 show_empty) {
	if (memory_table == NULL)
        memory_table = memory_hash_table_create(MEMORY_TABLE_INITIAL_LENGTH);
    
	size_t i = 0;
	MemoryList* list = NULL;
	MemoryNode* node = NULL;
	size_t n_lists = 0;
    
	log_info("INNER STATE");
	for (i = 0; i < memory_table->n_entries; ++i) {
		
		list = &(memory_table->entries[i]);
		node = list->first;
		
		if (show_entries == 1 && (show_empty == 1 || list->first != NULL)) 
            printf("[%4llu] ", i);
        
		if (list->first != NULL)
            n_lists++;
        
		if (show_entries == 1) {
			while (node != NULL) {
				printf("-> %p ", node->ptr);
				node = node->next;
			}
		}
		
        if (show_entries == 1 && (show_empty == 1 || list->first != NULL))
            printf("\n");
	}
    
	printf("TABLE LENGTH: %llu\nTABLE ENTRIES: %llu\nAverage List Length %f\n", memory_table->length, memory_table->n_entries, (float)memory_table->length / n_lists);
}

