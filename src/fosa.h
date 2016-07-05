#ifndef FOSA_H
#define FOSA_H

#include "fosa_string.h"
#include "fosa_pool.h"
#include "fosa_map.h"

struct fosa_endpoint_t;

typedef enum {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    OPTIONS,
    HEAD
} fosa_method_t;

static const char * fosa_http_status_lines[] = {

    "200 OK",
    "201 Created",
    "202 Accepted",
    NULL,  /* "203 Non-Authoritative Information" */
    "204 No Content",
    NULL,  /* "205 Reset Content" */
    "206 Partial Content",

#define FOSA_HTTP_STATUS_2XX_END 6
#define FOSA_HTTP_STATUS_3XX_OFFSET 300 + FOSA_HTTP_STATUS_2XX_END + 1

    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    NULL,  /* "305 Use Proxy" */
    NULL,  /* "306 unused" */
    "307 Temporary Redirect",

#define FOSA_HTTP_STATUS_3XX_END FOSA_HTTP_STATUS_2XX_END + 7
#define FOSA_HTTP_STATUS_4XX_OFFSET 400 + FOSA_HTTP_STATUS_3XX_END + 1

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

#define FOSA_HTTP_STATUS_4XX_END FOSA_HTTP_STATUS_3XX_END + 22
#define FOSA_HTTP_STATUS_5XX_OFFSET 500 + FOSA_HTTP_STATUS_4XX_END + 1

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

typedef struct fosa_conn_t {
    struct fosa_endpoint_t* endpoint;

    char req_protocol[14];
    char* req_path;
    char* req_segments[255];
    char* req_query_string;
    char* req_host;
    char* req_url;
    char* req_method;

    fosa_map_t req_headers;
    fosa_map_t res_headers;

    short res_status;
    char* res_body;
    void (*res_match)(struct fosa_conn_t*);

} fosa_conn_t;

static inline void fosa_conn_init(fosa_conn_t* conn) {
    fosa_map_init(&conn->req_headers, 10);
    fosa_map_init(&conn->res_headers, 10);
}

typedef void (fosa_plug_t)(fosa_conn_t*);

static inline size_t fosa_hash(const char* str) {
    return strlen(str) + (int)str[0];
}

typedef struct {
    short type;
    short flags;
    char* value;
} fosa_segment_match_rule_t;

typedef struct {
    fosa_segment_match_rule_t segments[20];
    size_t num_segments;
    char* route;
    void (*handler)(struct fosa_conn_t*);
    fosa_method_t method;
} fosa_match_handler_t;

typedef struct fosa_endpoint_t {
    unsigned short port;
    char* hostname;
    fosa_plug_t* plugs[20];
    fosa_match_handler_t routes[20];
} fosa_endpoint_t;

static inline void fosa_endpoint_init(fosa_endpoint_t* endpoint) {
    endpoint->port = 8080;
    endpoint->hostname = "0.0.0.0";
}

typedef void (fosa_match_t)(fosa_conn_t*);

static inline fosa_method_t fosa_parse_method_str(const char *method) {
    fosa_method_t m = GET;
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

void fosa_run(fosa_conn_t* conn);

void fosa_put_status(fosa_conn_t* conn, short status);
void fosa_put_header_i(fosa_conn_t* conn, const char* key, const uint32_t i);
void fosa_put_header(fosa_conn_t* conn, const char* key, const char *value);
void fosa_put_body(fosa_conn_t* conn, const char* str);

void fosa_match(fosa_endpoint_t* endpoint,
                const char* method,
                char* path,
                fosa_match_t* match);

void fosa_router(fosa_conn_t* conn);
void fosa_request_id(fosa_conn_t* conn);
void fosa_put_content_length(fosa_conn_t* conn);
void fosa_log_request(fosa_conn_t* conn);

void fosa_server_start(fosa_endpoint_t * endpoint);

#endif
