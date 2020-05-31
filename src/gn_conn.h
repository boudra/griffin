#ifndef GN_CONN
#define GN_CONN

#include "gn_map.h"

struct gn_endpoint_t;

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


#endif
