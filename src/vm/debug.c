#include "debug.h"

struct {
    char *str;
    debug_fn fn;
} dbgtable[] = {
    {"bt", stack_trace},
};

void start_debug(Frame *frame) {
    size_t a = 0;
    printf("(debug)>> ");
    ReadStatus status = intern_readline(512, &a, "", 0);

    stack_trace(frame);
}

void stack_trace(Frame *frame) {
    for(Frame *f = frame; f->prev; f = f->prev) {
        printf("%s()\tin\n", f->func_name);
    }
    puts("<global>");
}
