#include "griffin.h"

#include <assert.h>

void homepage(gn_conn_t* conn) {
    gn_put_body(conn, "Hola mon!");
    gn_put_status(conn, 200);
}

int main(int argc, char *argv[]) {

    gn_endpoint_t endpoint = { .plugs = { &gn_router } };

    gn_match(&endpoint, "get", "/", homepage);

    gn_conn_t conn = {
        .endpoint = &endpoint,
        .req_path = "/",
        .req_segments = { "" },
        .req_method = "get"
    };

    gn_run(&conn);

    assert(strcmp(conn.res_body, "Hola mon!") == 0);
    assert(conn.res_status == 200);
    assert(conn.res_match == homepage);

    return 0;
}
