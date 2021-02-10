#include "maxc.h"
#include "mlib.h"
#include "util.h"

extern Vector *gmodule;

void module_init() {
  gmodule = new_vector();

  std_init();
  file_init();
  str_init();
  list_init();
  dir_init();
  int_init();
}
