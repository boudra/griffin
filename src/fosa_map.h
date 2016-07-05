#ifndef FOSA_MAP_H
#define FOSA_MAP_H

#include "fosa_string.h"
#include <stddef.h>

typedef struct {
    char* key;
    void* value;
} fosa_map_kv_t;

typedef struct {
    fosa_map_kv_t* data;
    size_t size;
    size_t count;
} fosa_map_t;

static inline size_t fosa_map_hash(const char* cp, const size_t size) {
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash % size;
}

static inline const size_t fosa_map_find_index(fosa_map_t* map, const char* key) {
    size_t i = fosa_map_hash(key, map->size);
    while(map->data[i].key != NULL && strcmp(key, map->data[i].key) > 0) {
        printf("Collizione %ld\n", i);
        i = (i + 1) % map->size;
    }
    return i;
}

static inline void fosa_map_resize(fosa_map_t* map, const size_t size) {
    fosa_map_kv_t* data = map->data;
    map->data = malloc(sizeof(fosa_map_kv_t) * size);
    memset(map->data, 0, sizeof(fosa_map_kv_t) * size);
    memcpy(map->data, data, sizeof(fosa_map_kv_t) * map->size);
    map->size = size;
    free(data);
}

static inline void fosa_map_init(fosa_map_t* map, const size_t size) {
    map->size = 0;
    map->count = 0;
    map->data = NULL;
    fosa_map_resize(map, size);
}

static inline void fosa_map_put(fosa_map_t* map, const char* key, void* value) {
    fosa_map_kv_t* kv = &map->data[fosa_map_find_index(map, key)];
    if(kv->key == NULL) map->count++;
    kv->key = fosa_strdup(key);
    kv->value = value;
}

static inline void* fosa_map_get(fosa_map_t* map, const char* key) {
    return map->data[fosa_map_find_index(map, key)].value;
}

static inline void fosa_map_delete(fosa_map_t* map, const char* key) {
    fosa_map_kv_t* kv = &map->data[fosa_map_find_index(map, key)];
    if(kv->key != NULL) {
        kv->key = NULL;
        kv->value = NULL;
        map->count--;
    }
}

static inline void fosa_map_empty(fosa_map_t* map) {
    map->count = 0;
    memset(map->data, 0, sizeof(fosa_map_kv_t) * map->size);
}

static inline void fosa_map_destroy(fosa_map_t* map) {
    map->count = 0;
    map->size = 0;
    free(map->data);
    map->data = NULL;
}

#endif /* end of include guard: FOSA_MAP_H */
