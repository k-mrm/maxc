#include "maxc.h"

ReadStatus intern_readline(size_t alloc, size_t max, int *cursor) {
    ReadStatus status = {
        .err.eof = 0,
        .err.toolong = 0
    };
    char a[alloc];
    memset(a, 0, alloc);
    char last_char;
    char *str;
    *cursor = 0;

    while((last_char = getchar()) != '\n') {
        if(last_char == EOF) {
            status.err.eof = 1;
            goto err;
        }

        a[(*cursor)++] = last_char;

        if(max <= *cursor) {
            status.err.toolong = 1;
            goto err;
        }
    }

    str = malloc(sizeof(char) * (*cursor + 1));
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
