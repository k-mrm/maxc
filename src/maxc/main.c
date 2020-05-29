#include "maxc.h"
#include "error/error.h"

char *filename = NULL;
char *code;

int main(int argc, char **argv) {
  mxc_interp_open(argc, argv);

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
