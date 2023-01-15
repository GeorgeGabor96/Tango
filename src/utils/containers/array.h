#ifndef __UTILS_CONTAINERS_ARRAY_H__
#define __UTILS_CONTAINERS_ARRAY_H__

// NOTE: Be carefull if the @new_len < @current_len because it will only
// NOTE: copy @new_len elements from @array, which are not all
// NOTE: this will not cause any RUNTIME ERRORS or CRACHES but it will potentially
// NOTE: cause memory leaks
// NOTE: only use this feature if you know that the rest of the elments is garbage
internal void* array_resize(void* array, sz el_size, u32 current_len, u32 new_len);


#endif // __UTILS_CONTAINERS_ARRAY_H__
