#include "maxc.h"

char *intern_readline() {
    char a[1024] = {0};
    char last_char;
    char *str;
    int cursor = 0;

    while((last_char = getchar()) != '\n') {
        if(last_char == EOF) {
            return NULL;
        }

        a[cursor++] = last_char;
    }
}

void intern_abort() {
    fprintf(stderr, "aborted.\n");
    abort();
}
