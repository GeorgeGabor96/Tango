#include "containers/list.h"

// HELPER functions
internal inline Node*
node_create(void* element, size_t el_size) {
    size_t node_size = sizeof(Node) + el_size;
    Node* node = (Node*)memory_malloc(node_size, "node_create");
    check_memory(node);
    node->prev = NULL;
    node->next = NULL;
    memcpy(node + 1, element, el_size);
    return node;
    
    error:
    return NULL;
}


internal void
list_node_destroy(Node* node, size_t el_size) {
    node->prev = NULL;
    node->next = NULL;
    memset(node + 1, 0, el_size);
    memory_free(node);
}


internal void
list_unlink_node(List* list, Node* node) {
    if (list->first == NULL) {
        return;
    }
    if (list->first == list->last && list->first == node) {
        list->first = NULL;
        list->last = NULL;
    } else if (list->first == node) {
        list->first = list->first->next;
    } else if (list->last == node) {
        list->last = list->last->prev;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    node->prev = NULL;
    node->next = NULL;
}


// ACTUAL functions
internal List*
list_create(size_t el_size) {
    check(el_size != 0, "el_size is 0");
    
    List* list = (List*)memory_malloc(sizeof(List), "list_create");
    check_memory(list);
    list_init(list, el_size);
    return list;
    
    error:
    return NULL;
}


internal void
list_init(List* list, size_t el_size) {
    check(el_size != 0, "el_size is 0");
    list->first = NULL;
    list->last = NULL;
    list->el_size = el_size;
    error:
    return;
}


internal void
list_destroy(List* list, ResetFn* reset_fn) {
    check(list != NULL, "NULL list");
    
    list_reset(list, reset_fn);
    memory_free(list);
    
    error:
    return;
}


internal void
list_reset(List* list, ResetFn* reset_fn) {
    check(list != NULL, "NULL list");
    
    Node* iter = list->first;
    Node* aux = NULL;
    u8* node_data_p = NULL;
    while (iter != NULL) {
        node_data_p = (u8*)(iter + 1);
        if (reset_fn){
            reset_fn(node_data_p);
        }
        aux = iter;
        iter = iter->next;
        memory_free(aux);
    }
    error:
    return;
}


internal void
list_add_last(List* list, void* element) {
    check(list != NULL, "NULL list");
    check(element != NULL, "NULL element");
    
    Node* node = node_create(element, list->el_size);
    check(node != NULL, "NULL node");
    
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->prev = list->last;
        list->last->next = node;
        list->last = node;
    }
    
    error:
    return;
}


internal void
list_add_first(List* list, void* element) {
    check(list != NULL, "NULL list");
    check(element != NULL, "NULL element");
    
    Node* node = node_create(element, list->el_size);
    check(node != NULL, "NULL node");
    
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
    }
    
    error:
    return;
}


internal void
list_remove_first(List* list, void* element) {
    check(list != NULL, "NULL list");
    check(element != NULL, "NULL element");
    if (list->first == NULL) {
        return;
    }
    
    Node* node = list->first;
    list_unlink_node(list, node);
    
    memcpy(element, (void*)(node + 1), list->el_size);
    
    list_node_destroy(node, list->el_size);
    
    error:
    return;
}


internal void
list_remove_last(List* list, void* element) {
    check(list != NULL, "NULL list");
    check(element != NULL, "NULL element");
    if (list->first == NULL) {
        return;
    }
    
    Node* node = list->last;
    list_unlink_node(list, node);
    
    memcpy(element, (void*)(node + 1), list->el_size);
    
    list_node_destroy(node, list->el_size);
    
    error:
    return;
}


internal void
list_show(List* list, ShowFn* show_fn) {
    check(list != NULL, "NULL list");
    check(show_fn != NULL, "NULL show_fn");
    
    Node* iter = list->first;
    void* el_p = NULL;
    while (iter != NULL) {
        el_p = (void*)(iter + 1);
        show_fn(el_p);
        printf(" ");
        iter = iter->next;
    }
    printf("\n");
    
    error:
    return;
}


internal u32
list_is_empty(List* list) {
    check(list != NULL, "NULL list");
    if (list->first == NULL && list->last == NULL) return 1;
    error:
    return 0;
}
