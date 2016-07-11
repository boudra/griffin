#ifndef GN_ENDPOINT
#define GN_ENDPOINT

#include "gn_router.h"

typedef void (gn_plug_t)(gn_conn_t*);

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

#endif
