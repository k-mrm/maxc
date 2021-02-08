#include <stdio.h>
#include <stdlib.h>
#include "maxc.h"
#include "ast.h"
#include "bytecode.h"
#include "codegen.h"
#include "error/error.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "token.h"
#include "type.h"
#include "vm.h"
#include "object/object.h"
#include "literalpool.h"
#include "mlib.h"

const char *code;
const char *filename;

void mxc_open(int argc, char **argv) {
  setup_argv(argc, argv);
  module_init();
  sema_init();
  op_addr_table_init();
}

void mxc_close() {
  ;
}

int mxc_main_file(const char *fname) {
  char *src = read_file(fname);
  if(!src) {
    error("%s: cannot open file", fname);
    return 1;
  }
  filename = fname;
  code = src;

  Vector *token = lexer_run(src, fname);

#ifdef MXC_DEBUG
  tokendump(token);
  //printf(BOLD("--- lex: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  struct mparser *pstate = parser_run(token);
  if(pstate->err) {
    return 1;
  }

  Vector *ast = pstate->ast;

#ifdef MXC_DEBUG
  //printf(BOLD("--- parse: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  int ngvars = sema_analysis(ast);

#ifdef MXC_DEBUG
  //printf(BOLD("--- sema_analysis: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  struct cgen *cinfo = compile(ast, ngvars);

#ifdef MXC_DEBUG
  // printf(BOLD("--- compile: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

#ifdef MXC_DEBUG
  puts(BOLD("--- literal pool ---"));
  lpooldump(cinfo->ltable);

  puts(BOLD("--- codedump ---"));
  printf("iseq len: %d\n", cinfo->iseq->len);

  /*
  printf("\e[2m");
  for(size_t i = 0; i < cinfo->iseq->len;) {
    codedump(cinfo->iseq->code, &i, cinfo->ltable);
    puts("");
  }
  puts(STR_DEFAULT);
  */

  puts(BOLD("--- exec result ---"));
#endif

  vm_open(cinfo->iseq->code, cinfo->gvars, ngvars, cinfo->ltable, cinfo->d);
  int exitcode = vm_run();

  return exitcode;
}

