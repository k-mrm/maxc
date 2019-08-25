#include "frame.h"

Frame *New_Global_Frame(Bytecode *c) {
    Frame *f = malloc(sizeof(Frame));

    f->code = c->code;
    f->codesize = c->len;
    f->pc = 0;

    return f;
}

Frame *New_Frame(userfunction *u) {
    Frame *f = malloc(sizeof(Frame));

    f->code = u->code;
    f->codesize = u->codesize;
    f->lvars = New_Vector_With_Size(u->nlvars);
    f->pc = 0;
    f->nlvars = u->nlvars;

    return f;
}

void Delete_Frame(Frame *f) {
    Delete_Vector(f->lvars);

    free(f);
}
