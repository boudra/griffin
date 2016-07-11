#include "griffin.h"
#include "gn_http.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void gn_match(gn_endpoint_t* endpoint,
                const char* method,
                char* path,
                gn_match_t* match) {

    gn_match_handler_t* last = &endpoint->routes[0];

    while(last->handler != NULL) last++;

    last->handler = match;
    last->route = gn_strdup(path);
    last->method = gn_parse_method_str(method);

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

void gn_put_body(gn_conn_t* conn, const char* str) {
    conn->res_body = (char*)str;
}

void gn_put_header(gn_conn_t* conn, const char* key, const char *value) {
    gn_map_put(&conn->res_headers, key, gn_strdup(value));
}

void gn_put_header_i(gn_conn_t* conn, const char* key, const uint32_t i) {
    char* len = malloc(16);
    sprintf(len, "%d", i);
    gn_put_header(conn, key, len);
    free(len);
}

void gn_run(gn_conn_t* conn) {
    for(int i = 0; i < 20; i++) {
        if(conn->endpoint->plugs[i])
            conn->endpoint->plugs[i](conn);
    }
}

void gn_put_status(gn_conn_t* conn, short status) {
    conn->res_status = status;
}

void gn_log_request(gn_conn_t* conn) {
    printf("%s %s -> %d\n", conn->req_method, conn->req_path, conn->res_status);
}

void gn_router(gn_conn_t* conn) {
    gn_match_handler_t *handler = &conn->endpoint->routes[0];
    const gn_method_t method = gn_parse_method_str(conn->req_method);
    while(conn->res_match == NULL && handler->handler != NULL) {
        if(handler->method != method) continue;
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
