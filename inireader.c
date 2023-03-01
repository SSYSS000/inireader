#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "inireader.h"

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
    ssize_t cap;
    ssize_t line_len = 0;
    enum { CHUNK_SIZE = 256 };

    line = *lineptr;
    cap = line != NULL ? *n : 0;

    do {
        if (line_len + 2 >= cap) {
            if ((line = realloc(line, cap + CHUNK_SIZE)) == NULL) {
                errno = ENOMEM;
                return -1;
            }

            *lineptr = line;
            cap = *n = cap + CHUNK_SIZE;
        }

        if ((c = fgetc(stream)) != EOF) {
            line[line_len++] = c;
        }
    } while (c != EOF && c != delim);

    if (c == EOF && line_len == 0)
        return -1;

    line[line_len] = '\0';
    return line_len;
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
    return isalnum(c) || isspace(c) || c == '.';
}

static int is_key_name_char(int c)
{
    return isalnum(c) || isspace(c);
}

static int is_key_value_char(int c)
{
    return !is_comment_char(c) && c != '\n';
}

static char *trim(char *str)
{
    size_t len;

    while (isspace(*str))
        str++;

    len = strlen(str);

    while (len > 0 && isspace(str[len - 1]))
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

    /* Get to the first non-whitespace character. */
    while (isspace(*line))
        line++;

    if (IS_EMPTY_STR(line))
        return NULL;

    ptr = copy_string(&state->linebuf, &state->linebuf_cap, line);
    if (!ptr) {
        // no memory error
        return NULL;
    }

    if (is_comment_char(*ptr)) {
        obj->type = INI_COMMENT;
        obj->comment.whole = ptr;
    }
    else if (*ptr == '[') {
        endptr = ptr + 1;

        while (is_section_char(*endptr))
            endptr++;

        if (*endptr == ']') {
            *endptr = '\0';
            obj->type = INI_SECTION;
            obj->section.name = trim(ptr + 1);
        }
        else {
            goto out_missing;
        }
    }
    else {
        endptr = ptr;
        while (is_key_name_char(*endptr))
            endptr++;

        if (*endptr == '=') {
            *endptr = '\0';
            obj->key.name = trim(ptr);
            obj->key.value = trim(endptr + 1);
            obj->type = INI_KEY;
        }
        else {
            goto out_missing;
        }
    }

    return obj;

out_missing:
    return NULL;
}

static ssize_t get_next_line(struct ini_file *ini)
{
    ssize_t line_len = 0;

    line_len = getline(&ini->line, &ini->line_cap, ini->stream);

    if (line_len != -1) {
        ini->line_num++;
    }

    return line_len;
}

struct ini_object *ini_get_next_object(struct ini_file *ini)
{
    struct ini_object *obj = NULL;

    while (!ini->state.err && get_next_line(ini) != -1) {
        obj = ini_read_object(&ini->state, ini->line);
        if (obj)
            break;
    }

    return obj;
}

