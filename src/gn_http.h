#ifndef GN_HTTP
#define GN_HTTP

#include "griffin.h"

static const char * gn_http_status_lines[] = {

    "200 OK",
    "201 Created",
    "202 Accepted",
    NULL,  /* "203 Non-Authoritative Information" */
    "204 No Content",
    NULL,  /* "205 Reset Content" */
    "206 Partial Content",

#define GN_HTTP_STATUS_2XX_END 6
#define GN_HTTP_STATUS_3XX_OFFSET 300 + GN_HTTP_STATUS_2XX_END + 1

    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    NULL,  /* "305 Use Proxy" */
    NULL,  /* "306 unused" */
    "307 Temporary Redirect",

#define GN_HTTP_STATUS_3XX_END GN_HTTP_STATUS_2XX_END + 7
#define GN_HTTP_STATUS_4XX_OFFSET 400 + GN_HTTP_STATUS_3XX_END + 1

    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Not Allowed",
    "406 Not Acceptable",
    NULL,  /* "407 Proxy Authentication Required" */
    "408 Request Time-out",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    NULL,  /* "417 Expectation Failed" */
    NULL,  /* "418 unused" */
    NULL,  /* "419 unused" */
    NULL,  /* "420 unused" */
    "421 Misdirected Request",
    "422 Unprocessable Entity",

#define GN_HTTP_STATUS_4XX_END GN_HTTP_STATUS_3XX_END + 22
#define GN_HTTP_STATUS_5XX_OFFSET 500 + GN_HTTP_STATUS_4XX_END + 1

    /* NULL, */  /* "422 Unprocessable Entity" */
    /* NULL, */  /* "423 Locked" */
    /* NULL, */  /* "424 Failed Dependency" */

    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Temporarily Unavailable",
    "504 Gateway Time-out",
    NULL,        /* "505 HTTP Version Not Supported" */
    NULL,        /* "506 Variant Also Negotiates" */
    "507 Insufficient Storage",

    /* NULL, */  /* "508 unused" */
    /* NULL, */  /* "509 unused" */
    /* NULL, */  /* "510 Not Extended" */

};

void gn_start_server(gn_endpoint_t * endpoint, const char* bind_address, unsigned short port);

static inline char* gn_parse_header_head(char* buffer, char** method, char** path, char **query_string) {
    char *ptr = NULL;
    buffer = gn_skip_whitespace(buffer);
    *method = buffer;
    buffer = gn_skip_until_char(buffer, ' ');
    *buffer = '\0';
    ptr = *method;
    while(*ptr != '\0') {
        *ptr = *ptr + 32;
        ptr++;
    }
    buffer = gn_skip_whitespace(++buffer);
    *path = buffer;
    buffer = gn_skip_until_chars(buffer, "? ");
    if(*buffer == '?') {
        *query_string = buffer + 1;
        *buffer = '\0';
        buffer++;
    }
    buffer = gn_skip_until_char(buffer, ' ');
    *path = gn_strndup(*path, buffer - *path);
    *buffer = '\0';
    return gn_skip_until_next_line(++buffer);
}

static inline char* gn_parse_header(char* buffer, gn_map_t* headers) {
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
    gn_map_insert(headers, key, value);
    return gn_skip_until_next_line(++buffer);
}


#endif
