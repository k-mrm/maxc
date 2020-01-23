#include "maxc.h"

ReadStatus intern_readline(size_t alloc,
                           size_t *cursor,
                           char *end,
                           size_t end_len) {
    ReadStatus status = {
        .err.eof = 0,
        .err.toolong = 0
    };
    char a[alloc];
    memset(a, 0, alloc);
    char last_char;
    char *str;
    size_t max = alloc - end_len - 1;
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

    str = malloc(sizeof(char) * (*cursor + end_len + 1));
    sprintf(str, "%s%s", a, end);

    status.str = str;
    return status;

err:
    status.str = NULL;
    return status;
}

void *xmalloc(size_t n) {
    void *p = malloc(n);
    if(!p) {
        intern_die("No Memory Error");
    }
    return p;
}

void intern_die(char *msg) {
    fprintf(stderr, "die: %s\n", msg);
    exit(1);
}

void intern_abort() {
    fprintf(stderr, "aborted.\n");
    abort();
}
