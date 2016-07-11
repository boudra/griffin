#ifndef GN_HTTP
#define GN_HTTP

#include "griffin.h"

void gn_server_start(gn_endpoint_t * endpoint);

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
    gn_map_put(headers, key, value);
    return gn_skip_until_next_line(++buffer);
}


#endif
