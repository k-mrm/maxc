#include "maxc.h"

int mxc_main_repl() {
    printf("Welcome to maxc!\n");
    printf("maxc Version %s\n", "0.0.1");

    int code_idx;
    char last_char;

    for(;;) {
        char code[256] = {0};
        printf(">> ");

        code_idx = 0;

        while((last_char = getchar()) != '\n') {
            ;
        }

        if(strcmp(code, ":q") == 0) {
            puts("Good Bye");
            return 0;
        }

        printf("debug: %s\n", code);
    }

    return 0;
}
