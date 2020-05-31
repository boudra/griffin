#include <stdint.h>
#include <stdlib.h>

#include "gn_router.h"
#include "griffin.h"

#include "gn_map.h"

void gn_router(gn_conn_t* conn, void* opts) {
    gn_router_t* router = (gn_router_t*)opts;
    gn_match_handler_t *handler = &router->routes[0];

    const gn_method_t method = gn_parse_method_str(conn->req_method);
    while(conn->res_match == NULL && handler->handler != NULL) {
        if(handler->method != method) {
            handler++;
            continue;
        }

        size_t matches = 0;
        size_t i = 0;
        for(i = 0; conn->req_segments[i] != NULL; ++i);
        size_t req_segments_len = i;
        for(i = 0; i < handler->num_segments && i < req_segments_len; ++i) {
            gn_segment_match_rule_t *rule = &handler->segments[i];
            if(rule == NULL) break;
            else if(rule->type == 0 && strcmp(rule->value, conn->req_segments[i]) == 0) {
                matches++;
            } else if(rule->type == 1) {
                matches++;
            }
        }
        if((matches > 0 && matches == handler->num_segments) ||
           (req_segments_len + handler->num_segments) == 0) {
            handler->handler(conn, &conn->req_params);
            conn->res_match = handler->handler;
            break;
        }
        handler++;
    }
    if(conn->res_match == NULL) {
        gn_put_status(conn, 404);
    }
}

void gn_router_match(gn_router_t* router,
                gn_method_t method,
                char* path,
                gn_match_t* match) {

    gn_match_handler_t* last = &router->routes[0];

    while(last->handler != NULL) last++;

    last->handler = match;
    last->route = gn_strdup(path);
    last->method = (method);

    assert(path[0] == '/');
    size_t segment = 0;

    path++;

    while(*path != '\0' && segment < 8) {
        char* end_segment = gn_skip_until_char(path, '/');
        if(*path == ':') {
            last->segments[segment].type = 1;
            path++;
        } else {
            last->segments[segment].type = 0;
        }
        last->segments[segment].value = gn_strndup(path, end_segment - path);
        path = end_segment;
        ++segment;
    }

    last->num_segments = segment;
}


