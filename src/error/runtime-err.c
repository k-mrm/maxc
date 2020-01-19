#include "error/error.h"
#include "error/runtime-err.h"
#include "frame.h"
#include "object/object.h"

extern char *filename;

void mxc_raise_err(Frame *f, enum RuntimeErrType ty) {
    f->occurred_rterr.type = ty;
}

void raise_outofrange(Frame *f,
                      MxcObject *got,
                      MxcObject *len) {
    f->occurred_rterr.type = RTERR_OUTOFRANGE;
    f->occurred_rterr.args[0] = got;
    f->occurred_rterr.args[1] = len;
    f->occurred_rterr.argc = 2;
}

void runtime_error(RuntimeErr error) {
    switch(error.type) {
    case RTERR_NONEERR:
        /* unreachable */
        return;
    case RTERR_OUTOFRANGE:
        fprintf(stderr,
                "\e[31;1m[runtime error] \e[0m"
                "index out of range: got %ld but length is %ld",
                ((IntObject *)error.args[0])->inum,
                ((IntObject *)error.args[1])->inum);
        break;
    case RTERR_ZERO_DIVISION:
        fprintf(stderr,
                "\e[31;1m[runtime error] \e[0m"
                "division by zero");
        break;
    case RTERR_UNIMPLEMENTED:
        fprintf(stderr,
                "\e[31;1m[runtime error] \e[0m"
                "sorry. unimplemented");
        break;
    }

    if(filename)
        fprintf(stderr, "\n\e[1m in %s\e[0m\n", filename);
}
