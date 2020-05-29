#include "maxc.h"
#include "mlib.h"
#include "util.h"

Vector *Global_Cbltins;

void load_default_module() {
  MInterp *interp = get_interp();
  interp->module = new_vector();
  vec_push(interp->module, std_init());
}
