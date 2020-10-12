#include "maxc.h"
#include "mlib.h"
#include "util.h"

void register_module(MInterp *m, MxcModule *mod) {
  vec_push(m->module, mod);
}

void load_default_module(MInterp *interp) {
  interp->module = new_vector();
  std_init(interp);
  flib_init(interp);
  strlib_init(interp);
}
