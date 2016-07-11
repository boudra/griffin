#include "griffin.h"

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
#include <arpa/inet.h>

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

char* gn_parse_header(char* buffer, gn_map_t* headers) {
    char *ptr = NULL;
    char *key = NULL;
    char *value = NULL;
    buffer = gn_skip_whitespace(buffer);
    key = buffer;
    buffer = gn_skip_until_char(buffer, ':');
    *buffer = '\0';
    ptr = key;
    while(*ptr != '\0') {
        if(*ptr >= 'A' && *ptr <= 'Z') {
            *ptr = *ptr + 32;
        }
        ptr++;
    }
    buffer = gn_skip_whitespace(++buffer);
    value = buffer;
    buffer = gn_skip_until_eof(buffer);
    *buffer = '\0';
    gn_map_put(headers, key, value);
    return gn_skip_until_next_line(++buffer);
}

void gn_server_start(gn_endpoint_t * endpoint) {

    int create_socket, new_socket;
    socklen_t addrlen;
    const int bufsize = 1024;

    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0){
        printf("The socket was created\n");
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(endpoint->hostname);
    address.sin_port = htons(endpoint->port);

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

                gn_conn_t conn = {
                    .endpoint = endpoint,
                    .res_status = 200,
                    .res_match = NULL,
                    .req_segments = {0}
                };

                gn_conn_init(&conn);

                recv_parse_buffer =
                    gn_parse_header_head(
                        recv_parse_buffer,
                        &conn.req_method,
                        &conn.req_path,
                        &conn.req_query_string
                    );

                gn_put_req_path(&conn, conn.req_path);

                while(recv_parse_buffer != gn_skip_until_eof(recv_parse_buffer)) {
                    recv_parse_buffer =
                        gn_parse_header(
                            recv_parse_buffer,
                            &conn.req_headers
                        );
                    ++header_num;
                }

                gn_run(&conn);

                size_t status_line_offset = 0;

                if(conn.res_status >= 300) {
                status_line_offset = conn.res_status - GN_HTTP_STATUS_3XX_OFFSET;
            }

            if(conn.res_status >= 400) {
                status_line_offset = conn.res_status - GN_HTTP_STATUS_4XX_OFFSET;
            }

            if(conn.res_status >= 500) {
                status_line_offset = conn.res_status - GN_HTTP_STATUS_5XX_OFFSET;
            }

            send_buffer_ptr += sprintf(
                send_buffer_ptr,
                "HTTP/1.1 %s\r\n",
                gn_http_status_lines[status_line_offset]
            );

            for(size_t i = 0; i < conn.res_headers.size; ++i) {
                gn_map_kv_t* header = &conn.res_headers.data[i];
                if(header->key != NULL) {
                    send_buffer_ptr += sprintf(
                            send_buffer_ptr, "%s: %s\r\n",
                            header->key, (char*)header->value
                    );
                }
            }

            sprintf(send_buffer_ptr, "\r\n");
            write(new_socket, send_buffer, strlen(send_buffer));

            if(conn.res_body) {
                write(new_socket, conn.res_body, strlen(conn.res_body));
            }

            close(new_socket);
        }

    }

    close(create_socket);
}
