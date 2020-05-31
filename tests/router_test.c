#include "griffin.h"
#include "gn_router.h"

#include <assert.h>

void root(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Hola mon!");
    gn_put_status(conn, 200);
}

void say_hello(gn_conn_t* conn, gn_map_t* params) {
    const char* name = gn_map_get(params, "name");
    if(name != NULL) {
        puts(name);
    }

    gn_put_body(conn, "Hello world!");
    gn_put_status(conn, 200);
}

int main(int argc, char *argv[]) {
    gn_endpoint_t endpoint = {0};
    gn_router_t router = {0};

    gn_add_middleware(&endpoint, &gn_router, &router);

    gn_router_match(&router, GET, "/", root);
    /* gn_router_match(&router, GET, "/hello/name", say_hello); */

    gn_conn_t conn = gn_conn_create(&endpoint, "get", "/", NULL);
    gn_run(&conn);

    assert(conn.res_status == 200);
    assert(conn.res_match == root);
    assert(strcmp(conn.res_body, "Hola mon!") == 0);

    conn = gn_conn_create(&endpoint, "get", "/hello/moosh", NULL);
    /* gn_run(&conn); */

    assert(conn.res_status == 200);
    assert(conn.res_match == say_hello);
    assert(strcmp(conn.res_body, "Hello moosh!") == 0);

    conn = gn_conn_create(&endpoint, "get", "/test", NULL);
    gn_run(&conn);

    assert(conn.res_status == 404);

    return 0;
}
