# Griffin

Tiny HTTP server library in C, inspired by the design of [Plug](https://github.com/elixir-plug/plug);

I built this for fun and to learn the HTTP protocol and sockets in more depth, it has not ben tested in production.

## Tasks

- [x] Basic HTTP server
- [x] Basic routing and path parameter parsing
- [ ] HTTPS/TLS

## Example

```c
#include <griffin.h>
#include <gn_router.h>
#include <gn_http.h>

void root(gn_conn_t* conn, gn_map_t* params) {
    gn_put_body(conn, "Root");
    gn_put_status(conn, 200);
}

void say_hello(gn_conn_t* conn, gn_map_t* params) {
    const char* name = gn_map_get(params, "name");

    gn_put_body(conn, name);
    gn_put_status(conn, 200);
}

int main(int argc, char *argv[]) {
    gn_endpoint_t endpoint = {0};
    gn_router_t router = {0};

    gn_add_middleware(&endpoint, &gn_router, &router);

    gn_router_match(&router, GET, "/", root);
    gn_router_match(&router, GET, "/hello/:name", say_hello);
    
    gn_start_server(&endpoint, "0.0.0.0", 8080);

    return 0;
}

```
