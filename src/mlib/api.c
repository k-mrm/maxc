#include <stdarg.h>
#include "mlibapi.h"

int mlib_parse_arg(MxcValue *arg, int narg, ...) {
  va_list ap;
  va_start(ap, narg);
  for(int i = 0; i < narg; i++) {
    MxcValue *v = va_arg(ap, MxcValue *);
    *v = arg[i];
  }
}
