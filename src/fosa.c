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

void fosa_start_server(fosa_endpoint_t * endpoint) {

    int create_socket, new_socket;
    socklen_t addrlen;
    const int bufsize = 1024;

    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0){
        printf("The socket was created\n");
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int optval = 1;
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0) {
        printf("bind ok\n");
    }

    while (1) {

        if (listen(create_socket, 10) < 0) {
            printf("listen error\n");
            exit(1);
        }

        if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) {
            printf("accept error\n");
            exit(1);
        }

        {
            char recv_buffer[1024] = {0};
            char *recv_parse_buffer = &recv_buffer[0];
            char send_buffer[1024] = {0};
            char *send_buffer_ptr = &send_buffer[0];

            int header_num = 0;

            recv(new_socket, recv_buffer, bufsize, 0);

            fosa_conn_t conn = {
                .endpoint = endpoint,
                .res_headers = {{0}},
                .res_status = 200,
                .res_match = NULL,
                .req_segments = {0}
            };

            recv_parse_buffer =
                fosa_parse_header_head(
                    recv_parse_buffer,
                    &conn.req_method,
                    &conn.req_path,
                    &conn.req_query_string
                );

            size_t segment = 0;
            char *path = conn.req_path;

            assert(path[0] == '/');

            while(*path != '\0') {
                path++;
                char* end_segment = fosa_skip_until_char(path, '/');
                conn.req_segments[segment] = fosa_strndup(path, end_segment - path);
                path = end_segment;
                ++segment;
            }

            while(recv_parse_buffer != fosa_skip_until_eof(recv_parse_buffer)) {
                recv_parse_buffer =
                    fosa_parse_header(
                        recv_parse_buffer,
                        &conn.req_headers[header_num][0],
                        &conn.req_headers[header_num][1]
                    );
                ++header_num;
            }

            conn.req_headers[header_num][0] = NULL;
            conn.req_headers[header_num][1] = NULL;

            fosa_run(&conn);

            size_t status_line_offset = 0;

            if(conn.res_status >= 300) {
                status_line_offset = conn.res_status - FOSA_HTTP_STATUS_3XX_OFFSET;
            }

            if(conn.res_status >= 400) {
                status_line_offset = conn.res_status - FOSA_HTTP_STATUS_4XX_OFFSET;
            }

            if(conn.res_status >= 500) {
                status_line_offset = conn.res_status - FOSA_HTTP_STATUS_5XX_OFFSET;
            }

            send_buffer_ptr += sprintf(
                send_buffer_ptr,
                "HTTP/1.1 %s\r\n",
                fosa_http_status_lines[status_line_offset]
            );

            char** header = (char**)conn.res_headers[0];

            while(header[0] != NULL && header[1] != NULL) {
                send_buffer_ptr += sprintf(send_buffer_ptr, "%s: %s\r\n", header[0], header[1]);
                free(header[0]);
                free(header[1]);
                header += 2;
            }

            sprintf(send_buffer_ptr, "\r\n");
            write(new_socket, send_buffer, strlen(send_buffer));

            if(conn.res_body) {
                printf("%s\n", conn.res_body);
                write(new_socket, conn.res_body, strlen(conn.res_body));
            }

            close(new_socket);
        }

    }

    close(create_socket);
}
