#ifndef GN_ROUTER
#define GN_ROUTER

#include "gn_endpoint.h"

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

typedef struct {
    gn_match_handler_t routes[20];
} gn_router_t;

void gn_router(gn_conn_t* conn, void* opts);

void gn_router_match(gn_router_t* router,
                gn_method_t method,
                char* path,
                gn_match_t* match);

#endif
