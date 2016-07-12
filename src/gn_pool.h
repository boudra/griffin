#ifndef GN_POOL_H
#define GN_POOL_H

#include <stdint.h>
#include <assert.h>

struct gn_mem_pool_s;

typedef struct gn_mem_block_s {
    struct gn_mem_block_s* next;
    struct gn_mem_block_s* prev;
    struct gn_mem_block_s* first_child;
    struct gn_mem_block_s* parent;
    int occupied;
    size_t block_size;
    uint8_t data[];
} gn_mem_block_t;

#define get_block(ptr) (gn_mem_block_t*)((uint8_t*)ptr - sizeof(gn_mem_block_t))
#define get_data(ptr) (void*)((uint8_t*)ptr + sizeof(gn_mem_block_t))

static inline void* gn_alloc(void* ctx, const size_t block_size) {

    const size_t req_size = block_size + sizeof(gn_mem_block_t);

    if(ctx == NULL) {

        gn_mem_block_t* block = malloc((sizeof(gn_mem_block_t) * 2 ) + block_size);
        memset(block, 0, (sizeof(gn_mem_block_t) * 2) + block_size);

        block->occupied = 1;
        block->first_child = (gn_mem_block_t*)(block->data);
        block->first_child->block_size = block_size;
        block->first_child->parent = block;
        block->block_size = sizeof(gn_mem_block_t) + block_size;

        return block->data;

    }

    gn_mem_block_t* block = get_block(ctx);

    gn_mem_block_t* child = block->first_child;

    if(child == NULL && block->block_size > block_size) {
        child = (gn_mem_block_t*)block->data;
        memset(child, 0, sizeof(gn_mem_block_t));
        child->block_size = block->block_size - sizeof(gn_mem_block_t);
        child->parent = block;
        block->first_child = child;
    }

    while(child) {
        child = child->next;
    }

    child = block->first_child;

    while(child != NULL && (child->occupied || child->block_size < req_size)) {
        child = child->next;
    }

    assert(child != NULL && child->occupied == 0);

    if(child == NULL || child->occupied) {
    }

    if(child->block_size - block_size > sizeof(gn_mem_block_t)) {
        if(child->next && !child->next->occupied) {
            child->next->block_size += child->block_size - block_size;
            child->next = (gn_mem_block_t*)((uint8_t*)(child->next) - child->block_size);
        } else {
            gn_mem_block_t* next = (gn_mem_block_t*)(child->data + block_size);
            next->parent = child->parent;
            next->prev = child;
            next->occupied = 0;
            next->block_size = child->block_size - block_size;
            next->next = child->next;
            child->next = next;
        }
    }

    child->block_size = block_size;
    child->occupied = 1;


    return (child->data);
}

static inline void gn_free(void* ctx) {
    if(!ctx) return;
    gn_mem_block_t* block = get_block(ctx);

    if(!block->parent ||
       (block < block->parent && block > (gn_mem_block_t*)((uint8_t*)block->parent
                                                           + block->block_size))) {
        free(block);
    } else {
        if(block->prev && !block->prev->occupied) {
            block->prev->block_size += block->block_size;
            block->prev->next = block->next;
        } else if(block->next && !block->next->occupied) {
            block->block_size += block->next->block_size;
            block->occupied = 0;
            block->next = NULL;
        } else {
            block->occupied = 0;
        }
    }

}

#endif
