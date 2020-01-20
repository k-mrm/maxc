#include "maxc.h"

ReadStatus intern_readline() {
    ReadStatus status = {
        .err.eof = 0,
        .err.toolong = 0
    };
    char a[1024] = {0};
    char last_char;
    char *str;
    int cursor = 0;

    while((last_char = getchar()) != '\n') {
        if(last_char == EOF) {
            status.err.eof = 1;
            goto err;
        }

        a[cursor++] = last_char;

        if(cursor >= 1023) {
            status.err.toolong = 1;
            goto err;
        }
    }

    str = malloc(sizeof(char) * (cursor + 1));
    strcpy(str, a);

    status.str = str;
    return status;

err:
    status.str = "";
    return status;
}

void intern_abort() {
    fprintf(stderr, "aborted.\n");
    abort();
}
