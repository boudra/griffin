#include "fosa_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    fosa_map_t map;

    fosa_map_init(&map, 10);

    assert(map.data != NULL);
    assert(map.size == 10);
    assert(map.count == 0);

    fosa_map_put(&map, "a", "one");
    assert(strcmp("one", fosa_map_get(&map, "a")) == 0);

    fosa_map_put(&map, "a", "two");
    assert(strcmp("two", fosa_map_get(&map, "a")) == 0);

    fosa_map_put(&map, "b", "three");

    assert(map.count == 2);

    fosa_map_delete(&map, "b");

    assert(map.count == 1);

    return 0;
}
