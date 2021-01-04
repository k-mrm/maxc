#include "maxc.h"
#include "mlib.h"
#include "util.h"

Vector *gmodule;

void load_module(Vector *mods, MxcModule *m) {
  vec_push(mods, m);
}

void load_default_module() {
  load_module(gmodule, std_module());
  load_module(gmodule, flib_module());
  load_module(gmodule, strlib_module());
  load_module(gmodule, listlib_module());
}
