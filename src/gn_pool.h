#ifndef GN_POOL_H
#define GN_POOL_H

#include <stdint.h>

struct gn_mem_pool_s;

typedef struct gn_mem_block_s {
    struct gn_mem_block_s* next;
    struct gn_mem_block_s* prev;
    struct gn_mem_pool_s* parent;
    uint8_t data[];
} gn_mem_block_t;

typedef struct gn_mem_pool_s {
    gn_mem_block_t* first;
    size_t real_block_size;
} gn_mem_pool_t;

gn_mem_pool_t* gn_pool_create(const size_t element_size, const size_t num_blocks);
void* gn_pool_alloc(gn_mem_pool_t *p);
void gn_pool_free(void* ptr);

#endif
