#ifndef GN_ROUTER
#define GN_ROUTER

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


#endif
