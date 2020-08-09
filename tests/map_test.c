#include "gn_map.h"
#include "gn_pool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    gn_map_t map;

    gn_map_init(&map, 10);

    assert(map.data != NULL);
    assert(map.size == 10);
    assert(map.count == 0);

    gn_map_insert(&map, "a", "one");
    assert(strcmp("one", gn_map_find(&map, "a")) == 0);

    gn_map_insert(&map, "a", "two");
    assert(strcmp("two", gn_map_find(&map, "a")) == 0);
    assert(map.count == 1);

    gn_map_insert(&map, "b", "three");

    assert(map.count == 2);

    gn_map_erase(&map, "b");

    assert(map.count == 1);

    gn_map_clear(&map);

    assert(map.count == 0);
    assert(map.size == 10);

    gn_map_insert(&map, "b", "three");

    gn_map_destroy(&map);

    assert(map.count == 0);
    assert(map.size == 0);
    assert(map.data == NULL);

    return 0;
}
