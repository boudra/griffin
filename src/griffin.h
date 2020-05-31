#ifndef GN_H
#define GN_H

#include "gn_string.h"
#include "gn_pool.h"
#include "gn_map.h"
#include "gn_endpoint.h"

static inline void gn_put_req_path(gn_conn_t* conn, const char *path) {
    assert(path[0] == '/');
    gn_str_split(path + 1, '/', conn->req_segments);
    conn->req_path = gn_strdup(path);
}

static inline void gn_conn_init(gn_conn_t* conn) {
    gn_map_init(&conn->req_headers, 10);
    gn_map_init(&conn->res_headers, 10);
    gn_map_init(&conn->req_params, 20);
    gn_map_init(&conn->req_body_params, 10);
    gn_map_init(&conn->req_query_params, 10);
}

static inline gn_conn_t gn_conn_create(struct gn_endpoint_t *endpoint,
        const char *method, const char *path, gn_map_t *params) {
    gn_conn_t conn = { .endpoint = endpoint, .req_method = gn_strdup(method) };
    gn_conn_init(&conn);
    gn_put_req_path(&conn, path);
    return conn;
}

static inline gn_method_t gn_parse_method_str(const char *method) {
    gn_method_t m = GET;
    if(strcmp(method, "get") == 0) {
        m = GET;
    } else if(strcmp(method, "post") == 0) {
        m = POST;
    } else if(strcmp(method, "put") == 0) {
        m = PUT;
    } else if(strcmp(method, "patch") == 0) {
        m = PATCH;
    } else if(strcmp(method, "head") == 0) {
        m = HEAD;
    } else if(strcmp(method, "options") == 0) {
        m = OPTIONS;
    } else if(strcmp(method, "delete") == 0) {
        m = DELETE;
    }
    return m;
}

void gn_run(gn_conn_t* conn);

void gn_put_status(gn_conn_t* conn, short status);
void gn_put_header_i(gn_conn_t* conn, const char* key, const uint32_t i);
void gn_put_header(gn_conn_t* conn, const char* key, const char *value);
void gn_put_body(gn_conn_t* conn, const char* str);

void gn_request_id(gn_conn_t* conn);
void gn_put_content_length(gn_conn_t* conn);
void gn_log_request(gn_conn_t* conn);

#endif
