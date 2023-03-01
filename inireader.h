#ifndef JH_INIREADER_H
#define JH_INIREADER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ini_object_type {
    INI_SECTION,
    INI_KEY,
    INI_COMMENT
};

struct ini_comment {
    char *whole;
};

struct ini_section {
    char *name;
};

struct ini_key {
    char *name;
    char *value;
};

struct ini_object {
    union {
        struct ini_comment comment;
        struct ini_section section;
        struct ini_key key;
    };
    enum ini_object_type type;
};

enum ini_error_code {
    INI_OK = 0,
    INI_NO_MEM,
    INI_INVALID_SYM,
    INI_MISSING_SYM
};

struct ini_state {
    char *linebuf;
    size_t linebuf_cap;
    struct ini_object objectbuf;
    enum ini_error_code err;
    int err_index;
};

struct ini_file {
    FILE *stream;
    struct ini_state state;
    char *line;
    size_t line_cap;
    unsigned long line_num;
};

struct ini_object *ini_read_object(struct ini_state *state, const char *line);

struct ini_object *ini_get_next_object(struct ini_file *ini);

#ifdef __cplusplus
}
#endif

#endif /* JH_INIREADER_H */

