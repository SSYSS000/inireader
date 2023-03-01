#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "../inireader.h"

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
        fprintf(stderr, "bad stuff at %d of line %s", ini.state.err_index + 1,
                ini.line);
    }

    free(ini.state.linebuf);
    free(ini.line);

    return 0;
}

