#include "maxc.h"
#include "mlib.h"
#include "util.h"

void register_module(MxcModule *mod) {
  MInterp *interp = our_interp();
  vec_push(interp->module, mod);
}

void load_default_module() {
  MInterp *interp = our_interp();
  interp->module = new_vector();

  std_init();
  flib_init();
}
