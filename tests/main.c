#include "fosa.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

void homepage(fosa_conn_t* conn) {
    fosa_put_body(conn, "Hola mon!");
    fosa_put_status(conn, 200);
}

int main(int argc, char *argv[]) {

    fosa_endpoint_t endpoint = { .plugs = { &fosa_router } };

    fosa_match(&endpoint, "get", "/", homepage);

    fosa_conn_t conn = {
        .endpoint = &endpoint,
        .res_headers = {{0}},
        .req_path = "/",
        .req_segments = { "" },
        .req_method = "get"
    };

    fosa_run(&conn);

    assert(strcmp(conn.res_body, "Hola mon!") == 0);
    assert(conn.res_status == 200);
    assert(conn.res_match == homepage);

    return 0;
}
