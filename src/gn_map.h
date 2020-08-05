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
    unsigned size;
    unsigned count;
} gn_map_t;

#define HASH_PRIME 2000575757u

static inline const unsigned gn_map_get_hash(const char* cp, const size_t size);

static inline const size_t gn_map_get_index(const gn_map_t* map, const char* key);

static inline void gn_map_resize(gn_map_t* map, const size_t size);

static inline void gn_map_init(gn_map_t* map, const size_t size);

static inline void gn_map_insert(gn_map_t* map, const char* key, void* value);

static inline void* gn_map_find(gn_map_t* map, const char* key);

static inline void gn_map_extract(gn_map_t* map, const char* key);

static inline void gn_map_copy(gn_map_t* map, gn_map_t* new_map);

static inline void gn_map_destroy(gn_map_t* map);

static inline const unsigned gn_map_get_hash(const char* cp, const size_t size) {
    unsigned hash = 1;
    while (*cp) {
        hash *= (unsigned char) *cp;
        ++cp;
    }
    return hash % HASH_PRIME % size;
}

static inline const size_t gn_map_get_index(const gn_map_t* map, const char* key) {
    unsigned candidate = gn_map_get_hash(key, map->size);
    unsigned iterations_count = 0;
    while (map->data[candidate].key != NULL && strcmp(key, map->data[candidate].key) != 0 && iterations_count < map->size) {
        candidate = (candidate + 1) % map->size;
        ++iterations_count;
    }
    if (iterations_count >= map->size) {
        return map->size;
    }
    return candidate;
}

static inline void gn_map_resize(gn_map_t* map, const size_t new_size) {
    gn_map_t old_map = *map;
    gn_map_kv_t* data = map->data;
    map->data = calloc(new_size, sizeof(gn_map_kv_t));
    map->size = new_size;
    gn_map_copy(&old_map, map);
    map->size = new_size;
    gn_map_destroy(&old_map);
}

static inline void gn_map_init(gn_map_t* map, const size_t size) {
    map->size = 1;
    map->count = 0;
    map->data = calloc(1, sizeof(gn_map_kv_t));
    gn_map_resize(map, size);
}

static inline void gn_map_insert(gn_map_t* map, const char* key, void* value) {
    unsigned index = gn_map_get_index(map, key);
    if (index == map->size) {
            gn_map_resize(map, map->size * 2);
            index = gn_map_get_index(map, key);
    }
    if (map->data[index].key == NULL) {
        map->count += 1;    
    } else {
        free(map->data[index].key);
    }
    map->data[index].key = gn_strdup(key);
    map->data[index].value = value;
}

static inline void* gn_map_find(gn_map_t* map, const char* key) {
    unsigned index = gn_map_get_index(map, key);
    if (index == map->size) {
        return NULL;
    }
    if (map->data[index].key == NULL) {
        return NULL;
    }
    return map->data[index].value;
}

static inline void gn_map_erase(gn_map_t* map, const char* key) {
    unsigned index = gn_map_get_index(map, key);
    if (index == map->size || map->data[index].key == NULL) {
        return;
    }
    gn_map_kv_t* kv = &map->data[index];
    free(kv->key);
    kv->key = NULL;
    --map->count;
}

static inline void gn_map_clear(gn_map_t* map) {
    for (unsigned index = 0; index < map->size; ++index) {
        if (map->data[index].key != NULL) {
                free(map->data[index].key);
                map->data[index].key = NULL;
        }
    }
    map->count = 0;
}

static inline void gn_map_destroy(gn_map_t* map) {
    for (unsigned index = 0; index < map->size; ++index) {
        if (map->data[index].key != NULL) {
                free(map->data[index].key);
                map->data[index].key = NULL;
        }
    }
    map->count = 0;
    map->size = 0;
    free(map->data);
    map->data = NULL;
}

static inline void gn_map_copy(gn_map_t* src, gn_map_t* dest) {
    for (unsigned begin = 0; begin < src->size; ++begin) {
        if (src->data[begin].key != NULL) {
                gn_map_insert(dest, src->data[begin].key, src->data[begin].value);
        }
    }
}
#endif /* end of include guard: GN_MAP_H */
