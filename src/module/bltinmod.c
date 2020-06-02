#include "maxc.h"
#include "mlib.h"
#include "util.h"

Vector *Global_Cbltins;

void load_default_module() {
  MInterp *interp = our_interp();
  interp->module = new_vector();
  vec_push(interp->module, std_init());
}
