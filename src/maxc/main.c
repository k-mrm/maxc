#include "maxc.h"

int main(int argc, char **argv) {
  mxc_open(argc, argv);

  if(argc == 1) {
    return mxc_main_repl();
  }

  int err = mxc_main_file(argv[1]);

  mxc_close();

  return err;
}
