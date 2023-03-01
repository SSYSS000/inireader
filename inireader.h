/*
Copyright (c) 2023 SSYSS000

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef JH_INIREADER_H
#define JH_INIREADER_H

#include <stdio.h>

#if defined(_MSC_VER)
# define INIREADER_EXPORT __declspec(dllexport)
#else
# define INIREADER_EXPORT
#endif /* defined(_MSC_VER) */

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

INIREADER_EXPORT struct ini_object *ini_read_object(
        struct ini_state *state,
        const char *line);

INIREADER_EXPORT struct ini_object *ini_get_next_object(struct ini_file *ini);

#ifdef __cplusplus
}
#endif

#endif /* JH_INIREADER_H */

