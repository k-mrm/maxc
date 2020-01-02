#include "error/error.h"
#include "error/runtime-err.h"
#include "frame.h"

extern char *filename;

void mxc_raise_err(Frame *f, enum RuntimeErrType ty) {
    f->occurred_error = ty;
}

void runtime_error(enum RuntimeErrType ty) {
    char *errmsg;

    switch(ty) {
    case RTERR_NONEERR:
        /* unreachable */
        return;
    case RTERR_OUTOFRANGE:
        errmsg = "index out of range";
        break;
    case RTERR_ZERO_DIVISION:
        errmsg = "division by zero";
        break;
    case RTERR_UNIMPLEMENTED:
        errmsg = "sorry. unimplemented";
        break;
    }

    fprintf(stderr,
            "\e[31;1m[runtime error] \e[0m%s\n",
            errmsg);
    if(filename)
        fprintf(stderr, "\e[1m in %s\e[0m\n", filename);
}
