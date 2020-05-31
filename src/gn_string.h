#ifndef GN_STRING_H
#define GN_STRING_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <assert.h>
#include <stdint.h>

#define is_utf8(c) (((c) > 0x7f))

static inline const size_t gn_u8_num_bytes(const char *in) {
    size_t num = 1;
    const uint8_t first = *in;
    if(first >= 0xf0) {
        num = 4;
    } else if(first >= 0xe0) {
        num = 3;
    } else if(first >= 0xc0) {
        num = 2;
    }
    return num;
}

static inline const size_t gn_u32_num_bytes(const wchar_t in) {
    size_t num = 1;
    if(in >= 0x10000) {
        num = 4;
    } else if(in >= 0x800) {
        num = 3;
    } else if(in >= 0x80) {
        num = 2;
    }
    return num;
}

static inline const wchar_t gn_to_u32(const char *in) {
    const size_t bytes = gn_u8_num_bytes(in);
    wchar_t wc = (0x7f >> (bytes - 1 * !is_utf8(in[0]))) & in[0];
    wc = wc << ((bytes - 1) * 6);
    for(size_t i = 1; i < bytes; ++i) {
        wc += (in[i] & 0x3f) << ((bytes - i - 1) * 6);
    }
    return wc;
}

static inline const size_t gn_to_u8(wchar_t in, char *out) {
    const size_t num = gn_u32_num_bytes(in);
    out[0] = (in >> ((num - 1)*6));
    out[0] |= (0xff << (8 - num) & 0xff) * is_utf8(in);
    for(size_t i = 1; i < num; ++i) {
        out[i] = 0x80 + ((in >> ((num-i-1)*6)) & 0x3f);
    }
    return num;
}

static inline const char* gn_next_char(const char *in) {
    return in + gn_u8_num_bytes(in);
}

static inline void gn_to_upper(const char *in, char* out) {
    for(; *in; in = gn_next_char(in)) {
        out = out + gn_to_u8(towupper(gn_to_u32(in)), out);
    }
}

static inline const size_t gn_strlen(const char *s) {
    size_t chars = 0;
    for(; *s; s = gn_next_char(s)) chars++;
    return chars;
}

static inline const size_t gn_url_encode(const char* in, char *out) {
    const char *hex = "0123456789ABCDEF";
    const char *start = out;
    for(; *in; ++in) {
        if((*in >= 'a' && *in <= 'z') ||
           (*in >= 'A' && *in <= 'Z') ||
           (*in >= '0' && *in <= '9') ||
           (*in >= '0' && *in <= '9') ||
           *in == '-' || *in == '_' ||
           *in == '.' || *in == '!' ||
           *in == '~' || *in == '*'  ||
           *in == '\'' || *in == '(' ||
           *in == ')' || *in == '(') {
            *(out++) = *in;
        } else {
            *(out++) = '%';
            *(out++) = hex[(uint8_t)*in >> 4];
            *(out++) = hex[(uint8_t)*in & 15];
        }
    }
    *out = '\0';
    return (size_t)(out - start);
}

static inline const uint8_t gn_parse_hex_byte(const char in) {
    uint8_t c = 0;
    if(in <= '9') {
        c = (in - '0');
    } else {
        c = (in - 'A') + 10;
    }
    return c;
}

static inline const size_t gn_url_decode(const char* in, char *out) {
    const char *start = out;
    for(; *in; ++in) {
        if(*in == '%') {
            *(out++) = (gn_parse_hex_byte(*(in+1))<<4) + gn_parse_hex_byte(*(in+2));
            in += 2;
        } else if (*in == '+') {
            *(out++) = ' ';
        } else {
            *(out++) = *in;
        }
    }
    *out = '\0';
    return (size_t)(out - start);
}

static inline char* gn_strndup(const char *s, size_t len) {
    char *new = malloc(len + 1);
    if (new == NULL) return NULL;
    new[len] = '\0';
    return memcpy(new, s, len);
}

static inline char* gn_strdup(const char *s) {
    const size_t len = strlen(s);
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return memcpy(new, s, len);
}

static inline char* gn_skip_whitespace(char *str) {
    while((*str == ' ' || *str == '\n' || *str == '\r') && *str != '\0') str++;
    return str;
}

static inline char* gn_skip_until_char(char *str, char ch) {
    while(*str != ch && *str != '\0') str++;
    return str;
}

static inline char* gn_skip_until_not_char(char *str, char ch) {
    while(*str == ch && *str != '\0') str++;
    return str;
}

static inline char* gn_skip_until_chars(char *str, const char* ch) {
    while((strchr(ch, *str) == NULL) && *str != '\0') str++;
    return str;
}

static inline char* gn_skip_until_next_line(char *str) {
    while(*str != '\n' && *str != '\0') str++;
    if(*str != '\0') str++;
    return str;
}

static inline char* gn_skip_until_eof(char *str) {
    while(*str != '\n' && *str != '\0') str++;
    if(*(str - 1) == '\r') str--;
    return str;
}

static inline const size_t gn_str_split(const char *in, char delimiter, char **out) {
    size_t i = 0;
    while(*(in)) {
        char* end_part = gn_skip_until_char((char*)in, delimiter);
        out[i] = gn_strndup(in, (end_part - in) + 1);
        out[i][(end_part-in)] = '\0';
        in = end_part;
        ++i;
    }
    return i;
}

#endif

