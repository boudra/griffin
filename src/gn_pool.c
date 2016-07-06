#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "gn_pool.h"

gn_mem_pool_t* gn_pool_create(const size_t element_size, const size_t num_blocks) {

    const size_t real_block_size = sizeof(gn_mem_block_t) + element_size;

    gn_mem_pool_t *p = malloc(sizeof(gn_mem_pool_t) + (real_block_size * num_blocks));
    memset(p, 0, sizeof(gn_mem_pool_t) + (real_block_size * num_blocks));
    p->real_block_size = real_block_size;

    uint8_t *blocks =  ((uint8_t*)p) + sizeof(gn_mem_pool_t);
    uint8_t *last = blocks + (real_block_size * (num_blocks - 1));

    p->first = (gn_mem_block_t*)blocks;

    uint8_t* current = (uint8_t*)p->first;

    while(current <= last) {
        ((gn_mem_block_t*)current)->parent = p;
        ((gn_mem_block_t*)current)->prev = current == blocks ? NULL :
            (gn_mem_block_t*)(current - real_block_size);
        ((gn_mem_block_t*)current)->next = current == last ? NULL :
            (gn_mem_block_t*)(current + real_block_size);
        current += real_block_size;
    }

    return p;
}

void* gn_pool_alloc(gn_mem_pool_t *p) {
    if(p->first != NULL) {
        gn_mem_block_t* first = p->first;
        p->first = first->next;
        first->next = NULL;
        first->prev = NULL;
        return ((uint8_t*)first) + sizeof(gn_mem_block_t);
    } else {
        gn_mem_block_t *b = malloc(p->real_block_size);
        memset(b, 0, p->real_block_size);
        b->parent = p;
        return ((uint8_t*)b) + sizeof(gn_mem_block_t);
    }
}

void gn_pool_free(void *ptr) {
    gn_mem_block_t* b = (gn_mem_block_t*)((uint8_t*)ptr - sizeof(gn_mem_block_t));
    gn_mem_pool_t* p = b->parent;
    if(p->first == NULL) {
        b->next = NULL;
        b->prev = NULL;
        p->first = b;
        return;
    }
    gn_mem_block_t *child = p->first;
    while(child->next != NULL) child = child->next;
    child->next = b;
}
