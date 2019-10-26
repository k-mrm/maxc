#include "frame.h"

Frame *New_Global_Frame(Bytecode *c) {
    Frame *f = malloc(sizeof(Frame));

    f->prev = NULL;
    f->code = c->code;
    f->codesize = c->len;
    f->pc = 0;

    return f;
}

Frame *New_Frame(userfunction *u, Frame *prev) {
    Frame *f = malloc(sizeof(Frame));

    f->prev = prev;
    f->code = u->code;
    f->codesize = u->codesize;

    f->lvars = malloc(sizeof(struct MxcObject *) * u->nlvars);
    for(int i = 0; i < u->nlvars; ++i) {
        f->lvars[i] = NULL;
    }

    f->pc = 0;
    f->nlvars = u->nlvars;

    return f;
}

void Delete_Frame(Frame *f) {
    free(f->lvars);
    free(f);
}
