#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "pool.h"

fosa_mem_pool_t* fosa_pool_create(const size_t element_size, const size_t num_blocks) {

    const size_t real_block_size = sizeof(fosa_mem_block_t) + element_size;

    fosa_mem_pool_t *p = malloc(sizeof(fosa_mem_pool_t) + (real_block_size * num_blocks));
    memset(p, 0, sizeof(fosa_mem_pool_t) + (real_block_size * num_blocks));
    p->real_block_size = real_block_size;

    uint8_t *blocks =  ((uint8_t*)p) + sizeof(fosa_mem_pool_t);
    uint8_t *last = blocks + (real_block_size * (num_blocks - 1));

    p->first = (fosa_mem_block_t*)blocks;

    uint8_t* current = (uint8_t*)p->first;

    while(current <= last) {
        ((fosa_mem_block_t*)current)->parent = p;
        ((fosa_mem_block_t*)current)->prev = current == blocks ? NULL :
            (fosa_mem_block_t*)(current - real_block_size);
        ((fosa_mem_block_t*)current)->next = current == last ? NULL :
            (fosa_mem_block_t*)(current + real_block_size);
        current += real_block_size;
    }

    return p;
}

void* fosa_pool_alloc(fosa_mem_pool_t *p) {
    if(p->first != NULL) {
        fosa_mem_block_t* first = p->first;
        p->first = first->next;
        first->next = NULL;
        first->prev = NULL;
        return ((uint8_t*)first) + sizeof(fosa_mem_block_t);
    } else {
        fosa_mem_block_t *b = malloc(p->real_block_size);
        memset(b, 0, p->real_block_size);
        b->parent = p;
        return ((uint8_t*)b) + sizeof(fosa_mem_block_t);
    }
}

void fosa_pool_free(void *ptr) {
    fosa_mem_block_t* b = (fosa_mem_block_t*)((uint8_t*)ptr - sizeof(fosa_mem_block_t));
    fosa_mem_pool_t* p = b->parent;
    if(p->first == NULL) {
        b->next = NULL;
        b->prev = NULL;
        p->first = b;
        return;
    }
    fosa_mem_block_t *child = p->first;
    while(child->next != NULL) child = child->next;
    child->next = b;
}
