internal void
_memory_add_block(Memory* memory, sz size) {
    MemoryBlock* block = NULL;
    check(memory != NULL, "memory is NULL");

    sz block_size = size > memory->block_size ? size : memory->block_size;

    block = (MemoryBlock*)memory_malloc(sizeof(*block) + block_size, "_memory_block_create");
    check_memory(block);

    block->memory_start = (u8*)(block + 1);
    block->memory_end = block->memory_start + block_size;
    block->next = NULL;

    if (memory->first_block == NULL) {
        memory->first_block = block;
        memory->last_block = block;
    } else {
        memory->last_block->next = block;
        memory->last_block = block;
    }

    memory->current_block = block;
    ++(memory->n_blocks);
    memory->current_adr = memory->current_block->memory_start;

    error:
    return;
}


internal Memory*
memory_create(sz block_size, b32 allow_bigger) {
    Memory* memory = NULL;

    check(block_size != 0, "block_size is 0");
    memory = (Memory*) memory_calloc(1, sizeof(*memory), "memory_create");
    check_memory(memory);
    memory->block_size = block_size;
    memory->current_adr = NULL;
    memory->current_block = NULL;
    memory->first_block = NULL;
    memory->last_block = NULL;
    memory->n_blocks = 0;
    memory->allow_bigger = allow_bigger;

    return memory;

    error:
    if (memory != NULL) {
        if (memory->first_block != NULL) memory_free(memory->first_block);
        memory_free(memory);
    }

    return NULL;
}


internal void
memory_destroy(Memory* memory) {
    check(memory != NULL, "memory is NULL");

    MemoryBlock* it = memory->first_block;
    MemoryBlock* aux = NULL;

    while (it != NULL) {
        aux = it;
        it = it->next;
        memset(aux, 0, sizeof(*aux) + memory->block_size);
        memory_free(aux);
    }
    memset(memory, 0, sizeof(*memory));
    memory_free(memory);

    error:
    return;
}


internal void
memory_clear(Memory* memory) {
    check(memory != NULL, "memory is NULL");

    memory->current_block = memory->first_block;
    memory->current_adr = memory->first_block->memory_start;

    error:
    return;
}


internal void*
memory_push(Memory* memory, sz size) {
    u8* memory_adr = NULL;

    check(memory != NULL, "memory is NULL");
    check(size != 0, "size is 0");
    if_check(memory->allow_bigger == FALSE,
          size <= memory->block_size,
          "size %zu is bigger than the block size %zu. Allow bigger is FALSE",
          size, memory->block_size);
    // NOTE: On this machine if memory is not aligned based on the type, its okay, but
    // NOTE: pretty sure that the CPU is doing more that it should
    // NOTE: for this, aligned everything to sizeof(void*) so that there is no problem

    // TODO: we could make this better by considering types that are lower than the size_t
    // TODO: lets see if it will make sense to do this
    sz reminder = size % sizeof(void*);
    if (reminder != 0) {
        size += sizeof(void*) - reminder;
    }

    // NOTE: Check if memory is empty
    if (memory->first_block == NULL) {
        _memory_add_block(memory, size);
        memory_adr = memory->current_adr;
        memory->current_adr = memory_adr + size;
    }
    else {
        b32 found_block = FALSE;
        // NOTE: Try to find a block that fits the size
        while (memory->current_block != NULL) {
            if (memory->current_adr + size <= memory->current_block->memory_end) {
                found_block = TRUE;
                break;
            }
            memory->current_block = memory->current_block->next;
            if (memory->current_block)
                memory->current_adr = memory->current_block->memory_start;
        }
        if (!found_block) {
            // NOTE: add a block that is big enough
            _memory_add_block(memory, size);
        }

        memory_adr = memory->current_adr;
        memory->current_adr += size;
    }

    error:
    return (void*)memory_adr;
}


internal void*
memory_push_zero(Memory* memory, sz size) {
    void* adr = memory_push(memory, size);
    // TODO: this will not set exactly everything to 0, because we don't consider the reminder from alignment
    if (adr) memset(adr, 0, size);
    return adr;
}