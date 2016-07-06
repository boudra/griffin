#ifndef GN_MAP_H
#define GN_MAP_H

#include "gn_string.h"
#include <stddef.h>

typedef struct {
    char* key;
    void* value;
} gn_map_kv_t;

typedef struct {
    gn_map_kv_t* data;
    size_t size;
    size_t count;
} gn_map_t;

static inline size_t gn_map_hash(const char* cp, const size_t size) {
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash % size;
}

static inline const size_t gn_map_find_index(gn_map_t* map, const char* key) {
    size_t i = gn_map_hash(key, map->size);
    while(map->data[i].key != NULL && strcmp(key, map->data[i].key) > 0) {
        printf("Collizione %ld\n", i);
        i = (i + 1) % map->size;
    }
    return i;
}

static inline void gn_map_resize(gn_map_t* map, const size_t size) {
    gn_map_kv_t* data = map->data;
    map->data = malloc(sizeof(gn_map_kv_t) * size);
    memset(map->data, 0, sizeof(gn_map_kv_t) * size);
    memcpy(map->data, data, sizeof(gn_map_kv_t) * map->size);
    map->size = size;
    free(data);
}

static inline void gn_map_init(gn_map_t* map, const size_t size) {
    map->size = 0;
    map->count = 0;
    map->data = NULL;
    gn_map_resize(map, size);
}

static inline void gn_map_put(gn_map_t* map, const char* key, void* value) {
    gn_map_kv_t* kv = &map->data[gn_map_find_index(map, key)];
    if(kv->key == NULL) map->count++;
    kv->key = gn_strdup(key);
    kv->value = value;
}

static inline void* gn_map_get(gn_map_t* map, const char* key) {
    return map->data[gn_map_find_index(map, key)].value;
}

static inline void gn_map_delete(gn_map_t* map, const char* key) {
    gn_map_kv_t* kv = &map->data[gn_map_find_index(map, key)];
    if(kv->key != NULL) {
        kv->key = NULL;
        kv->value = NULL;
        map->count--;
    }
}

static inline void gn_map_empty(gn_map_t* map) {
    map->count = 0;
    memset(map->data, 0, sizeof(gn_map_kv_t) * map->size);
}

static inline void gn_map_destroy(gn_map_t* map) {
    map->count = 0;
    map->size = 0;
    free(map->data);
    map->data = NULL;
}

#endif /* end of include guard: GN_MAP_H */
