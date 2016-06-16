#include "fosa.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

void fosa_match(fosa_endpoint_t* endpoint,
                const char* method,
                char* path,
                fosa_match_t* match) {

    fosa_match_handler_t* last = &endpoint->routes[0];

    while(last->handler != NULL) last++;

    last->handler = match;
    last->route = fosa_strdup(path);
    last->method = fosa_parse_method_str(method);

    assert(path[0] == '/');
    size_t segment = 0;

    while(*path != '\0' && segment < 8) {
        path++;
        char* end_segment = fosa_skip_until_char(path, '/');
        if(*path == ':') {
            last->segments[segment].type = 1;
            path++;
        } else {
            last->segments[segment].type = 0;
        }
        last->segments[segment].value = fosa_strndup(path, end_segment - path);
        path = end_segment;
        ++segment;
    }

    last->num_segments = segment;

}

void fosa_put_body(fosa_conn_t* conn, const char* str) {
    conn->res_body = (char*)str;
}

void fosa_put_header(fosa_conn_t* conn, const char* key, const char *value) {
    const char** last = (const char**)conn->res_headers[0];
    while(last[0] != NULL && last[1] != NULL) {
        last += 2;
    }
    last[0] = fosa_strdup(key);
    last[1] = fosa_strdup(value);
}

void fosa_put_header_i(fosa_conn_t* conn, const char* key, const uint32_t i) {
    char* len = malloc(16);
    sprintf(len, "%d", i);
    fosa_put_header(conn, key, len);
    free(len);
}

void fosa_run(fosa_conn_t* conn) {
    for(int i = 0; i < 20; i++) {
        if(conn->endpoint->plugs[i])
            conn->endpoint->plugs[i](conn);
    }
}

void fosa_put_status(fosa_conn_t* conn, short status) {
    conn->res_status = status;
}

void fosa_log_request(fosa_conn_t* conn) {
    printf("%s %s -> %d\n", conn->req_method, conn->req_path, conn->res_status);
}

void fosa_router(fosa_conn_t* conn) {
    fosa_match_handler_t *handler = &conn->endpoint->routes[0];
    const fosa_method_t method = fosa_parse_method_str(conn->req_method);
    while(conn->res_match == NULL && handler->handler != NULL) {
        if(handler->method != method) continue;
        size_t matches = 0;
        size_t i = 0;
        for(i = 0; i < handler->num_segments && conn->req_segments[i] != NULL; ++i) {
            fosa_segment_match_rule_t *rule = &handler->segments[i];
            if(rule == NULL) break;
            else if(rule->type == 0 && strcmp(rule->value, conn->req_segments[i]) == 0) {
                matches++;
            } else if(rule->type == 1) {
                matches++;
            }
        }
        if(matches > 0 && matches == handler->num_segments) {
            handler->handler(conn);
            conn->res_match = handler->handler;
            break;
        }
        handler++;
    }
    if(conn->res_match == NULL) {
        fosa_put_status(conn, 404);
    }
}

void fosa_put_content_length(fosa_conn_t* conn) {
    const uint32_t content_length = conn->res_body ? strlen(conn->res_body) : 0;
    fosa_put_header_i(conn, "Content-Length", content_length);
}

void fosa_request_id(fosa_conn_t* conn) {
    static uint32_t request_id = 0;

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    fosa_put_header(conn, "Server", "Fosa 0.1");
    fosa_put_header(conn, "Content-Type", "text/html; charset=UTF-8");
    fosa_put_header(conn, "Cache-Control", "private, max-age=0, no-cache");
    fosa_put_header(conn, "Date", buffer);
    fosa_put_header_i(conn, "X-Request-Id", ++request_id);

}

