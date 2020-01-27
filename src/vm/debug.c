#include "debug.h"
#include "ast.h"

struct {
    char *str;
    debug_fn fn;
} dbgtable[] = {
    {"bt", stack_trace},
    {"localvars", local_vars},
};

debug_fn search_table(char *input) {
    int ndtable = sizeof(dbgtable) / sizeof(dbgtable[0]);

    for(int i = 0; i < ndtable; i++) {
        if(strcmp(dbgtable[i].str, input) == 0) {
            return dbgtable[i].fn;
        }
    }

    return NULL;
}

void start_debug(Frame *frame) {
    printf("breakpoint at \n");
    printf("maxc debug mode\n");
    
    for(;;) {
        printf("(debug)>> ");
        size_t a = 0;
        ReadStatus status = intern_readline(512, &a, "", 0);
        if(strcmp(status.str, "exit") == 0) {
            return;
        }
        debug_fn fn = search_table(status.str);
        if(!fn) {
            puts("unknown command");
            goto end;            
        }

        fn(frame);
end:
        free(status.str);
    }
}

void stack_trace(Frame *frame) {
    for(Frame *f = frame; f->prev; f = f->prev) {
        printf("in\t%s()\n", f->func_name);
    }
    puts("in\t<global>");
}

void local_vars(Frame *frame) {
    for(size_t i = 0; i < frame->nlvars; ++i) {
        NodeVariable *cur = (NodeVariable *)frame->lvar_info->vars->data[i];
        printf("%s:\t", cur->name);
        printf("%s\n", OBJIMPL(frame->lvars[i])
                            ->tostring(frame->lvars[i])
                            ->str);

    }
}
