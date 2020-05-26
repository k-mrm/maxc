#include "maxc.h"
#include "error/error.h"

char *filename = NULL;
char *code;
MxcArg mxc_args;

extern int errcnt;

int main(int argc, char **argv) {
  Interp interp;
  mxc_interp_open(&interp, argc, argv);

  if(argc == 1) {
    return mxc_main_repl();
  }
  filename = argv[1];

  code = read_file(filename);
  if(!code) {
    error("%s: cannot open file", filename);
    return 1;
  }

  return mxc_main(code, filename);
}
