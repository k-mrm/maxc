#include "maxc.h"

int main(int argc, char **argv) {
  MInterp *i = mxc_open(argc, argv);

  if(argc == 1) {
    return mxc_main_repl(i);
  }

  int err = mxc_main_file(i, argv[1]);

  mxc_close(i);

  return err;
}
