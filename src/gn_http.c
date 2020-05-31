#include "gn_http.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

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

    #ifdef SO_REUSE_PORT
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    #endif

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
