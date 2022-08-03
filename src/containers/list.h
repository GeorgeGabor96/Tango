/* date = July 31st 2022 5:56 pm */

#ifndef __CONTAINERS_LIST_H__
#define __CONTAINERS_LIST_H__

#include "common.h"

// NOTE: Probably I will not use this in code that needs to be very fast,
// NOTE: so no init/reset methods needed

typedef struct Node {
    struct Node* prev;
    struct Node* next;
    // NOTE: the content is put directly after the node
} Node;


typedef struct List {
    Node* first;
    Node* last;
    size_t el_size;
} List;


internal List* list_create(size_t el_size);
internal void list_destroy(List* list, ResetFn* reset_fn);

internal void list_init(List* list, size_t el_size);
internal void list_reset(List* list, ResetFn* reset_fn);

internal void list_add_last(List* list, void* element);
internal void list_add_first(List* list, void* element);

internal void list_remove_first(List* list, void* element);
internal void list_remove_last(List* list, void* element);

internal void list_show(List* list, ShowFn* show_fn);
internal u32 list_is_empty(List* list);

internal void list_unlink_node(List* list, Node* node);

// NOTE: not sure about these yet, need some concrete use cases
/*internal void list_remove_node(List* list, Node* node);

internal Node* list_find_fn(List* list, CompareFn compare_fn);
internal Node* list_remove_fn(List* list, CompareFn compare_fn);
*/

#endif // __CONTAINERS_LIST_H__
