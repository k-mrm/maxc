#include <stdlib.h>
#include <string.h>

#include "object/object.h"
#include "frame.h"
#include "error/error.h"

Frame *new_global_frame(Bytecode *c, int ngvar) {
    Frame *f = malloc(sizeof(Frame));
    f->prev = NULL;
    f->func_name = "<global>";
    f->code = c ? c->code : NULL;
    f->codesize = c ? c->len : 0;
    f->pc = 0;
    f->gvars = malloc(sizeof(MxcValue) * ngvar);
    f->ngvars = ngvar;
    for(int i = 0; i < ngvar; ++i) {
        f->gvars[i] = mval_invalid;
    }
    f->lvars = NULL;
    f->nlvars = 0;
    f->stackptr = malloc(sizeof(MxcValue) * 1024);
    f->stackbase = f->stackptr;
    memset(f->stackptr, 0, sizeof(MxcValue) * 1024);
    f->occurred_rterr.type = RTERR_NONEERR; 

    return f;
}

Frame *new_frame(userfunction *u, Frame *prev) {
    Frame *f = malloc(sizeof(Frame));
    f->prev = prev;
    f->func_name = u->name;
    f->code = u->code;
    f->codesize = u->codesize;
    f->lvar_info = u->var_info;
    f->lvars = malloc(sizeof(MxcValue) * u->nlvars);
    for(int i = 0; i < u->nlvars; ++i) {
        f->lvars[i] = mval_invalid;
    }
    f->ngvars = prev->ngvars;
    f->gvars = prev->gvars;
    f->pc = 0;
    f->nlvars = u->nlvars;
    f->stackptr = prev->stackptr;
    f->stackbase = prev->stackbase;

    return f;
}

void delete_frame(Frame *f) {
    free(f->lvars);
    free(f);
}
