#ifndef GN_H
#define GN_H

#include "gn_string.h"
#include "gn_pool.h"
#include "gn_map.h"

struct gn_endpoint_t;

typedef enum {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    OPTIONS,
    HEAD
} gn_method_t;

static const char * gn_http_status_lines[] = {

    "200 OK",
    "201 Created",
    "202 Accepted",
    NULL,  /* "203 Non-Authoritative Information" */
    "204 No Content",
    NULL,  /* "205 Reset Content" */
    "206 Partial Content",

#define GN_HTTP_STATUS_2XX_END 6
#define GN_HTTP_STATUS_3XX_OFFSET 300 + GN_HTTP_STATUS_2XX_END + 1

    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    NULL,  /* "305 Use Proxy" */
    NULL,  /* "306 unused" */
    "307 Temporary Redirect",

#define GN_HTTP_STATUS_3XX_END GN_HTTP_STATUS_2XX_END + 7
#define GN_HTTP_STATUS_4XX_OFFSET 400 + GN_HTTP_STATUS_3XX_END + 1

    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Not Allowed",
    "406 Not Acceptable",
    NULL,  /* "407 Proxy Authentication Required" */
    "408 Request Time-out",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    NULL,  /* "417 Expectation Failed" */
    NULL,  /* "418 unused" */
    NULL,  /* "419 unused" */
    NULL,  /* "420 unused" */
    "421 Misdirected Request",
    "422 Unprocessable Entity",

#define GN_HTTP_STATUS_4XX_END GN_HTTP_STATUS_3XX_END + 22
#define GN_HTTP_STATUS_5XX_OFFSET 500 + GN_HTTP_STATUS_4XX_END + 1

    /* NULL, */  /* "422 Unprocessable Entity" */
    /* NULL, */  /* "423 Locked" */
    /* NULL, */  /* "424 Failed Dependency" */

    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Temporarily Unavailable",
    "504 Gateway Time-out",
    NULL,        /* "505 HTTP Version Not Supported" */
    NULL,        /* "506 Variant Also Negotiates" */
    "507 Insufficient Storage",

    /* NULL, */  /* "508 unused" */
    /* NULL, */  /* "509 unused" */
    /* NULL, */  /* "510 Not Extended" */

};

typedef struct gn_conn_t {
    struct gn_endpoint_t* endpoint;

    char req_protocol[14];
    char* req_path;
    char* req_segments[255];
    char* req_query_string;
    char* req_host;
    char* req_url;
    char* req_method;

    gn_map_t req_headers;
    gn_map_t res_headers;

    gn_map_t req_params;
    gn_map_t req_body_params;
    gn_map_t req_query_params;

    short res_status;
    char* res_body;
    void (*res_match)(struct gn_conn_t*, gn_map_t*);

} gn_conn_t;

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

typedef void (gn_plug_t)(gn_conn_t*);

static inline size_t gn_hash(const char* str) {
    return strlen(str) + (int)str[0];
}

typedef void (gn_match_t)(gn_conn_t*, gn_map_t*);

typedef struct {
    short type;
    short flags;
    char* value;
} gn_segment_match_rule_t;

typedef struct {
    gn_segment_match_rule_t segments[20];
    size_t num_segments;
    char* route;
    gn_match_t* handler;
    gn_method_t method;
} gn_match_handler_t;

typedef struct gn_endpoint_t {
    unsigned short port;
    char* hostname;
    gn_plug_t* plugs[20];
    gn_match_handler_t routes[20];
} gn_endpoint_t;

static inline void gn_endpoint_init(gn_endpoint_t* endpoint) {
    endpoint->port = 8080;
    endpoint->hostname = "0.0.0.0";
}

static inline gn_method_t gn_parse_method_str(const char *method) {
    gn_method_t m = GET;
    if(strcmp(method, "get") != 0) {
        m = GET;
    } else if(strcmp(method, "post") != 0) {
        m = POST;
    } else if(strcmp(method, "put") != 0) {
        m = PUT;
    } else if(strcmp(method, "patch") != 0) {
        m = PATCH;
    } else if(strcmp(method, "head") != 0) {
        m = HEAD;
    } else if(strcmp(method, "options") != 0) {
        m = OPTIONS;
    } else if(strcmp(method, "delete") != 0) {
        m = DELETE;
    }
    return m;
}

void gn_run(gn_conn_t* conn);

void gn_put_status(gn_conn_t* conn, short status);
void gn_put_header_i(gn_conn_t* conn, const char* key, const uint32_t i);
void gn_put_header(gn_conn_t* conn, const char* key, const char *value);
void gn_put_body(gn_conn_t* conn, const char* str);

void gn_match(gn_endpoint_t* endpoint,
                const char* method,
                char* path,
                gn_match_t* match);

void gn_router(gn_conn_t* conn);
void gn_request_id(gn_conn_t* conn);
void gn_put_content_length(gn_conn_t* conn);
void gn_log_request(gn_conn_t* conn);

#endif
