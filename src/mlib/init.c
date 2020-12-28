#include "maxc.h"
#include "mlib.h"
#include "util.h"

void register_module(Vector *mods, MxcModule *m) {
  vec_push(mods, m);
}

void load_default_module(Vector *mods) {
  load_module(mods, std_module());
  load_module(mods, flib_module());
  load_module(mods, strlib_module());
  load_module(mods, listlib_module());
}
