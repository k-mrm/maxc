#include <string.h>
#include <stdlib.h>

#include "ast.h"
#include "debug.h"
#include "object/object.h"
#include "internal.h"
#include "object/strobject.h"

struct {
    char *str;
    debug_fn fn;
} dbgtable[] = {
    {"bt", stack_trace},
    {"localvars", local_vars},
    {"help", debug_help},
};

debug_fn search_table(char *input) {
    static int ndtable = sizeof(dbgtable) / sizeof(dbgtable[0]);

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

void debug_help(Frame *frame) {
    INTERN_UNUSE(frame);

    puts("help page: maxc debug mode");
    puts("\nbt:\tdump backtrace of all stack frames");
    puts("\nlocalvars:\tdump local variables");
}

void local_vars(Frame *frame) {
    if(!frame->prev) {
        puts("Here is <global>. Please do `globalvars` to watch global variables.");
        return;
    }
    for(size_t i = 0; i < frame->nlvars; ++i) {
        NodeVariable *cur = (NodeVariable *)frame->lvar_info->vars->data[i];
        printf("%s:\t", cur->name);
        printf("%s\n", ostr(mval2str(frame->lvars[i]))->str);
    }
}
