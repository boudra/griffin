#include <griffin.h>
#include <gn_router.h>
#include <gn_http.h>

void root(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Root");
    gn_put_status(conn, 200);
}

void say_hello(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Hello world!");
    gn_put_status(conn, 200);
}

int main(int argc, char *argv[]) {
    gn_endpoint_t endpoint = {0};
    gn_router_t router = {0};

    gn_add_middleware(&endpoint, &gn_router, &router);

    gn_router_match(&router, GET, "/", root);
    gn_router_match(&router, GET, "/hello", say_hello);

    gn_start_server(&endpoint, "0.0.0.0", 8080);

    return 0;
}
