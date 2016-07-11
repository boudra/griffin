#include "gn_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <locale.h>

int main(int argc, char *argv[]) {
    const char *url = "=?/&%€Á";
    char out[200];

    gn_url_encode(url, out);
    assert(strcmp(out, "%3D%3F%2F%26%25%E2%82%AC%C3%81") == 0);

    gn_url_decode("%3D%3F%2F%26%25%E2%82%AC%C3%81", out);
    assert(strcmp(out, url) == 0);

    return 0;
}
