#include "maxc.h"

extern int errcnt;
extern char *filename;

int mxc_main_repl() {
    printf("Welcome to maxc repl mode!\n");
    printf("maxc Version %s\n", "0.0.1");

    filename = "<stdin>";

    int cursor = 0;
    char last_char;

    char repl_code[1024] = {0};

    for(;;) {
        printf(">> ");

        memset(repl_code, 0, 1024);
        cursor = 0;

        while((last_char = getchar()) != '\n') {
            if(last_char == EOF) return 0;

            repl_code[cursor++] = last_char;
        }

        if(strcmp(repl_code, ":q") == 0) {
            puts("Good Bye");
            return 0;
        }

        printf("debug: %s\n", repl_code);
        int exitcode = mxc_main(repl_code);
    }

    return 0;
}
