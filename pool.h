#include <stdint.h>

struct fosa_mem_pool_s;

typedef struct fosa_mem_block_s {
    struct fosa_mem_block_s* next;
    struct fosa_mem_block_s* prev;
    struct fosa_mem_pool_s* parent;
    uint8_t data[];
} fosa_mem_block_t;

typedef struct fosa_mem_pool_s {
    fosa_mem_block_t* first;
    size_t real_block_size;
} fosa_mem_pool_t;

fosa_mem_pool_t* fosa_pool_create(const size_t element_size, const size_t num_blocks);
void* fosa_pool_alloc(fosa_mem_pool_t *p);
void fosa_pool_free(void* ptr);
