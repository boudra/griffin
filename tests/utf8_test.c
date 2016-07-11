#include "gn_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <locale.h>

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    char out[200] = {0};

    assert(gn_u8_num_bytes("€") == 3);
    assert(gn_u8_num_bytes("Á") == 2);
    assert(gn_u8_num_bytes("#") == 1);

    assert(gn_u32_num_bytes(L"\u20AC"[0]) == 3);
    assert(gn_u32_num_bytes(L"\u00C1"[0]) == 2);
    assert(gn_u32_num_bytes(L"#"[0]) == 1);

    assert(gn_to_u32("€") == L"\u20AC"[0]);
    assert(gn_to_u32("Á") == L"\u00C1"[0]);
    assert(gn_to_u32("#") == L"#"[0]);

    assert(gn_to_u8(L"\u20AC"[0], out) == 3);
    assert(strcmp(out, "€") == 0);

    const char* in = "codificación de correos electrónicos y páginas web";

    assert(gn_strlen(in) == 50);

    gn_to_upper(in, out);
    assert(strcmp(out, "CODIFICACIÓN DE CORREOS ELECTRÓNICOS Y PÁGINAS WEB") == 0);

    return 0;
}
