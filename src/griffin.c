#include "griffin.h"
#include "gn_http.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void gn_put_body(gn_conn_t* conn, const char* str) {
    conn->res_body = (char*)str;
}

void gn_put_header(gn_conn_t* conn, const char* key, const char *value) {
    gn_map_insert(&conn->res_headers, key, gn_strdup(value));
}

void gn_put_header_i(gn_conn_t* conn, const char* key, const uint32_t i) {
    char* len = malloc(16);
    sprintf(len, "%d", i);
    gn_put_header(conn, key, len);
    free(len);
}

void gn_run(gn_conn_t* conn) {
    for(int i = 0; i < 20; i++) {
        if(conn->endpoint->middlewares[i].fun != NULL) {
            conn->endpoint->middlewares[i].fun(
                    conn,
                    conn->endpoint->middlewares[i].options
            );
        }
    }
}

void gn_put_status(gn_conn_t* conn, short status) {
    conn->res_status = status;
}

void gn_log_request(gn_conn_t* conn) {
    printf("%s %s -> %d\n", conn->req_method, conn->req_path, conn->res_status);
}

void gn_put_content_length(gn_conn_t* conn) {
    const uint32_t content_length = conn->res_body ? strlen(conn->res_body) : 0;
    gn_put_header_i(conn, "Content-Length", content_length);
}

void gn_request_id(gn_conn_t* conn) {
    static uint32_t request_id = 0;

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    gn_put_header(conn, "Server", "Gn 0.1");
    gn_put_header(conn, "Content-Type", "text/html; charset=UTF-8");
    gn_put_header(conn, "Cache-Control", "private, max-age=0, no-cache");
    gn_put_header(conn, "Date", buffer);
    gn_put_header_i(conn, "X-Request-Id", ++request_id);

}
