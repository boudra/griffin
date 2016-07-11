#include "griffin.h"

#include <assert.h>

void root(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Hola mon!");
    gn_put_status(conn, 200);
}

void root2(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Hello world!");
    gn_put_status(conn, 200);
}

int main(int argc, char *argv[]) {

    gn_endpoint_t endpoint = { .plugs = { &gn_router } };

    gn_match(&endpoint, "get", "/", root);
    gn_match(&endpoint, "get", "/hello", root2);

    gn_conn_t conn = gn_conn_create(&endpoint, "get", "/", NULL);
    gn_run(&conn);

    assert(conn.res_status == 200);
    assert(conn.res_match == root);
    assert(strcmp(conn.res_body, "Hola mon!") == 0);

    conn = gn_conn_create(&endpoint, "get", "/hello", NULL);
    gn_run(&conn);

    assert(conn.res_status == 200);
    assert(conn.res_match == root2);
    assert(strcmp(conn.res_body, "Hello world!") == 0);

    conn = gn_conn_create(&endpoint, "get", "/test", NULL);
    gn_run(&conn);

    assert(conn.res_status == 404);

    return 0;
}
