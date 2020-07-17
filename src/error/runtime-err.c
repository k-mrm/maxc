#include "error/error.h"
#include "error/runtime-err.h"
#include "frame.h"
#include "object/object.h"
#include "object/intobject.h"

extern char *filename;

void mxc_raise_err(MContext *f, enum RuntimeErrType ty) {
  f->occurred_rterr.type = ty;
}

void raise_outofrange(MContext *f,
    MxcValue got,
    MxcValue len) {
  f->occurred_rterr.type = RTERR_OUTOFRANGE;
  f->occurred_rterr.args[0] = got;
  f->occurred_rterr.args[1] = len;
  f->occurred_rterr.argc = 2;
}

void runtime_error(MContext *f) {
  switch(f->occurred_rterr.type) {
    case RTERR_NONEERR:
      /* unreachable */
      return;
    case RTERR_OUTOFRANGE:
      log_error("\e[31;1m[runtime error] \e[0m"
          "index out of range: got %ld but length is %ld",
          f->occurred_rterr.args[0].num,
          f->occurred_rterr.args[1].num);
      break;
    case RTERR_ZERO_DIVISION:
      log_error("\e[31;1m[runtime error] \e[0m"
          "division by zero");
      break;
    case RTERR_ASSERT:
      log_error("\e[31;1m[runtime error] \e[0m"
          "assertion failed");
      break;
    case RTERR_UNIMPLEMENTED:
      log_error("\e[31;1m[runtime error] \e[0m"
          "sorry. unimplemented");
      break;
  }

  if(filename) {
    log_error("\n\e[1m in %s::%s\e[0m\n",
        filename,
        f->func_name);
  }
}
