#ifndef GN_ENDPOINT
#define GN_ENDPOINT

#include "gn_conn.h"

typedef enum {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    OPTIONS,
    HEAD
} gn_method_t;

typedef void (gn_middleware_fun_t)(gn_conn_t*, void*);

typedef struct {
    gn_middleware_fun_t* fun;
    void* options;
} gn_middleware_t;

typedef struct gn_endpoint_t {
    unsigned short port;
    char* hostname;
    gn_middleware_t middlewares[20];
} gn_endpoint_t;

static inline void gn_endpoint_init(gn_endpoint_t* endpoint) {
    endpoint->port = 8080;
    endpoint->hostname = "0.0.0.0";
}

static inline void gn_add_middleware(gn_endpoint_t* endpoint, gn_middleware_fun_t* fun, void* options) {
    endpoint->middlewares[0].fun = fun;
    endpoint->middlewares[0].options = options;
}

#endif
