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

void homepage(fosa_conn_t* conn) {
    fosa_put_body(conn, "Hola mon!");
    fosa_put_status(conn, 200);
}

int main(int argc, char *argv[]) {

    fosa_mem_pool_t* pool = fosa_pool_create(sizeof(int), 2048);

    int *x = NULL;
    for(int i = 0; i < 2048; i++) {
        x = fosa_pool_alloc(pool);
        *x = i;
    }

    fosa_pool_free(x);
    fosa_pool_alloc(pool);

    fosa_endpoint_t endpoint = {
        .port = 8080,
        .hostname = "*",
        .routes = {{{{0}}}},
        .plugs = {
            &fosa_router,
            &fosa_put_content_length,
            &fosa_request_id,
            &fosa_log_request
        }
    };

    fosa_match(&endpoint, "get", "/", homepage);
    fosa_match(&endpoint, "get", "/:any/test/hola/hue", homepage);

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
                .endpoint = &endpoint,
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

    return 0;
}
