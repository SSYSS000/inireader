#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "../inireader.h"

static const char *get_error_msg(enum ini_error_code err);
static void highlight_error(const char *line, int index);

int main(int argc, char *argv[])
{
    struct ini_file ini = {stdin};
    struct ini_object *obj;

    while ((obj = ini_get_next_object(&ini)) != NULL) {
        switch (obj->type) {
        case INI_SECTION:
            printf("SECTION: %s\n", obj->section.name);
            break;

        case INI_KEY:
            printf("    KEY: %s = %s\n", obj->key.name, obj->key.value);
            break;

        case INI_COMMENT:
            printf("COMMENT: %s\n", obj->comment.whole);
            break;

        default:
            break;
        }
    }
    
    if (ini.state.err) {
        fprintf(stderr, "error: %s at line %lu\n",
                get_error_msg(ini.state.err), ini.line_num);
        highlight_error(ini.line, ini.state.err_index);
    }

    free(ini.state.linebuf);
    free(ini.line);

    return 0;
}

static const char *get_error_msg(enum ini_error_code err)
{
    if (err == INI_MISSING_SYM)
        return "Missing symbol";
    else if (err == INI_INVALID_SYM)
        return "Invalid symbol";
    else if (err == INI_NO_MEM)
        return strerror(ENOMEM);
    else
        return "Unhandled error";
}

static void highlight_error(const char *line, int index)
{
    int i;

    fputs(line, stderr);
    for (i = 0; i < index; ++i) {
        if (isspace(line[i]))
            fputc(line[i], stderr);
        else
            fputc(' ', stderr);
    }

    fputc('^', stderr);
    fputc('\n', stderr);
}

