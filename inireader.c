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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "inireader.h"

#if defined(_MSC_VER)
# define restrict __restrict
typedef long long ssize_t;
#endif /* defined(_MSC_VER) */

#define IS_EMPTY_STR(str) ((str)[0] == '\0')

#ifndef HAVE_GETLINE
# define HAVE_GETLINE \
    defined(_GNU_SOURCE) || \
    (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
#endif

#ifndef USE_GETLINE_REIMPL
# define USE_GETLINE_REIMPL 1
#endif

#if USE_GETLINE_REIMPL
# define getdelim jh_getdelim
# define getline jh_getline

static ssize_t jh_getdelim(
        char **restrict lineptr,
        size_t *restrict n,
        int delim,
        FILE *restrict stream)
{
    int c;
    char *line;
    size_t cap;
    size_t line_len = 0;
    enum { CHUNK_SIZE = 256 };

    line = *lineptr;
    cap = line != NULL ? *n : 0;

    do {
        if (line_len + 2 >= cap) {
            cap += CHUNK_SIZE;
            if ((line = realloc(line, cap)) == NULL) {
                errno = ENOMEM;
                return -1;
            }

            *lineptr = line;
            *n = cap;
        }

        if ((c = fgetc(stream)) != EOF) {
            line[line_len++] = c;
        }
    } while (c != EOF && c != delim);

    if (c == EOF && line_len == 0)
        return -1;

    line[line_len] = '\0';
    return (ssize_t) line_len;
}

static ssize_t jh_getline(
        char **restrict lineptr,
        size_t *restrict n,
        FILE *restrict stream)
{
    return jh_getdelim(lineptr, n, '\n', stream);
}

#endif /* USE_GETLINE_REIMPL */

static int is_comment_char(int c)
{
    return c == '#' || c == ';';
}

static int is_section_char(int c)
{
    return !(c == '\0' || c == ']');
}

static int is_key_name_char(int c)
{
    return !(c == '\0' || c == '=');
}

static char *skip(const char *string, int (*test)(int))
{
    while (test((unsigned char) *string))
        string++;

    /* Cast: caller's responsibility not to write to read-only memory. */
    return (char *) string;
}

static int str_is_space(const char *string)
{
    string = skip(string, isspace);
    return IS_EMPTY_STR(string);
}

static char *trim(char *str)
{
    size_t len;

    str = skip(str, isspace);
    len = strlen(str);

    while (len > 0 && isspace((unsigned char) str[len - 1]))
        len--;

    str[len] = '\0';

    return str;
}

static char *copy_string(char **dest, size_t *dest_cap, const char *string)
{
    char *copy;
    size_t cap;
    size_t string_len;

    copy = *dest;
    cap = copy ? *dest_cap : 0;
    string_len = strlen(string);

    if (cap < string_len + 1) {
        cap = string_len + 101;
        copy = realloc(copy, cap);
    }

    if (copy) {
        memcpy(copy, string, string_len + 1);
        *dest = copy;
        *dest_cap = cap;
    }

    return copy;
}

struct ini_object *ini_read_object(struct ini_state *state, const char *line)
{
    struct ini_object *obj = &state->objectbuf;
    char *ptr;
    char *endptr;

    if (state->err || str_is_space(line))
        return NULL;

    ptr = copy_string(&state->linebuf, &state->linebuf_cap, line);
    if (!ptr) {
        state->err = INI_NO_MEM;
        return NULL;
    }

    ptr = trim(ptr);

    if (is_comment_char((unsigned char)*ptr)) {
        /* Comment */
        obj->type = INI_COMMENT;
        obj->comment.whole = ptr;
    }
    else if (*ptr == '[') {
        /* Section */
        endptr = skip(ptr + 1, is_section_char);

        if (*endptr == ']') {
            *endptr = '\0';
            obj->type = INI_SECTION;
            obj->section.name = trim(ptr + 1);
        }
        else if (*endptr == '\0') {
            state->err = INI_MISSING_SYM;
        }
        else {
            state->err = INI_INVALID_SYM;
        }
    }
    else {
        /* Key */
        endptr = skip(ptr, is_key_name_char);

        if (*endptr == '=') {
            *endptr = '\0';
            obj->key.name = trim(ptr);
            obj->key.value = trim(endptr + 1);
            obj->type = INI_KEY;
        }
        else if (*endptr == '\0') {
            state->err = INI_MISSING_SYM;
        }
        else {
            state->err = INI_INVALID_SYM;
        }
    }

    if (state->err) {
        state->err_index = endptr - state->linebuf;
        return NULL;
    }

    return obj;
}

static ssize_t get_next_line(struct ini_file *ini)
{
    ssize_t line_len = 0;

    errno = 0;
    line_len = getline(&ini->line, &ini->line_cap, ini->stream);

    if (line_len == -1) {
        if (errno == ENOMEM) {
           ini->state.err = INI_NO_MEM; 
        }
    }
    else {
        ini->line_num++;
    }

    return line_len;
}

struct ini_object *ini_get_next_object(struct ini_file *ini)
{
    struct ini_object *obj = NULL;

    while (!obj && !ini->state.err && get_next_line(ini) != -1) {
        obj = ini_read_object(&ini->state, ini->line);
    }

    return obj;
}

