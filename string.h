#include <string.h>

char* fosa_strndup(const char *s, size_t len) {
    char *new = malloc(len + 1);
    if (new == NULL) return NULL;
    new[len] = '\0';
    return memcpy(new, s, len);
}

char* fosa_strdup(const char *s) {
    const size_t len = strlen(s);
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return memcpy(new, s, len);
}

char* fosa_skip_whitespace(char *str) {
    while((*str == ' ' || *str == '\n' || *str == '\r') && *str != '\0') str++;
    return str;
}

char* fosa_skip_until_char(char *str, char ch) {
    while(*str != ch && *str != '\0') str++;
    return str;
}

char* fosa_skip_until_not_char(char *str, char ch) {
    while(*str == ch && *str != '\0') str++;
    return str;
}

char* fosa_skip_until_chars(char *str, const char* ch) {
    while((strchr(ch, *str) == NULL) && *str != '\0') str++;
    return str;
}

char* fosa_skip_until_next_line(char *str) {
    while(*str != '\n' && *str != '\0') str++;
    if(*str != '\0') str++;
    return str;
}

char* fosa_skip_until_eof(char *str) {
    while(*str != '\n' && *str != '\0') str++;
    if(*(str - 1) == '\r') str--;
    return str;
}

char* fosa_parse_header(char* buffer, char** key, char** value) {
    char *ptr = NULL;
    buffer = fosa_skip_whitespace(buffer);
    *key = buffer;
    buffer = fosa_skip_until_char(buffer, ':');
    *buffer = '\0';
    ptr = *key;
    while(*ptr != '\0') {
        if(*ptr >= 'A' && *ptr <= 'Z') {
            *ptr = *ptr + 32;
        }
        ptr++;
    }
    buffer = fosa_skip_whitespace(++buffer);
    *value = buffer;
    buffer = fosa_skip_until_eof(buffer);
    *buffer = '\0';
    return fosa_skip_until_next_line(++buffer);
}

char* fosa_parse_header_head(char* buffer, char** method, char** path, char **query_string) {
    char *ptr = NULL;
    buffer = fosa_skip_whitespace(buffer);
    *method = buffer;
    buffer = fosa_skip_until_char(buffer, ' ');
    *buffer = '\0';
    ptr = *method;
    while(*ptr != '\0') {
        *ptr = *ptr + 32;
        ptr++;
    }
    buffer = fosa_skip_whitespace(++buffer);
    *path = buffer;
    buffer = fosa_skip_until_chars(buffer, "? ");
    if(*buffer == '?') {
        *query_string = buffer + 1;
        *buffer = '\0';
        buffer++;
    }
    buffer = fosa_skip_until_char(buffer, ' ');
    *path = fosa_strndup(*path, buffer - *path);
    printf("PATH:%s\n", *path);
    *buffer = '\0';
    return fosa_skip_until_next_line(++buffer);
}

