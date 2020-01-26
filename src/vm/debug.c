#include "debug.h"

struct {
    char *str;
    debug_fn fn;
} dbgtable[] = {
    {"bt", stack_trace},
};

void start_debug(Frame *frame) {
    printf("breakpoint at \n");
    printf("maxc debug mode\n");
    
    for(;;) {
        printf("(debug)>> ");
        size_t a = 0;
        ReadStatus status = intern_readline(512, &a, "", 0);

        stack_trace(frame);

        free(status.str);
    }
}

void stack_trace(Frame *frame) {
    for(Frame *f = frame; f->prev; f = f->prev) {
        printf("in\t%s()\n", f->func_name);
    }
    puts("in\t<global>");
}
