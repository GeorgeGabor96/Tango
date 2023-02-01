#ifndef __UTILS_MEMORY_H__
#define __UTILS_MEMORY_H__

#ifdef MEMORY_MANAGE
// overide the normal memory management function to save aditional information
internal void* memory_malloc(size_t size, char* desc);
internal void* memory_calloc(size_t nitems, size_t size, char* desc);
internal void* memory_realloc(void* ptr, size_t size, char* desc);
internal void memory_free(void* ptr);

// for debuging to see how the memory structure is filled
internal void memory_show_inner_state(u32 show_entries, u32 show_empty);

// number of memory allocations that are currently not freed
internal size_t memory_get_n_blocks();

// number of bytes currently not freed
internal size_t memory_get_size();

// TRUE if allocations were freed
internal b32 memory_is_empty();

// Shows information about every memory allocations not freed
internal void memory_report();

#else

#define memory_malloc(s_data, desc) malloc(s_data)
#define memory_calloc(n_elem, s_elem, desc) calloc(n_elem, s_elem)
#define memory_realloc(p, s_data, desc) realloc(p, s_data)
#define memory_free(p) free(p)

#define memory_show_inner_state(a, b)
#define memory_get_n_blocks() 0
#define memory_get_size() 0
#define memory_is_empty() TRUE
#define memory_report()

#endif // MEMORY_MANAGE

#endif // __UTILS_MEMORY_H__

