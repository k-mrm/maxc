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

MInterp *mxc_open(int argc, char **argv) {
  MInterp *m = malloc(sizeof(MInterp));
  m->argc = argc;
  m->argv = argv;
  m->errcnt = 0;
  load_default_module(m);
  sema_init(m);

  return m;
}

void mxc_close(MInterp *m) {
  free(m);
}

int mxc_main_file(MInterp *interp, const char *fname) {
  const char *src = read_file(fname);
  if(!src) {
    error("%s: cannot open file", fname);
    return 1;
  }
  filename = fname;
  code = src;

  Vector *token = lexer_run(src, fname);

#ifdef MXC_DEBUG
  tokendump(token);
  printf(BOLD("--- lex: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  if(interp->errcnt) {
    return 1;
  }

  Vector *AST = parser_run(token);

#ifdef MXC_DEBUG
  printf(BOLD("--- parse: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  int ngvars = sema_analysis(AST);

#ifdef MXC_DEBUG
  printf(BOLD("--- sema_analysis: %s ---\n"),
      interp->errcnt ? "failed" : "success");
#endif

  if(interp->errcnt) {
    fprintf(stderr, BOLD("\n%d %s generated\n"),
        interp->errcnt, interp->errcnt >= 2 ? "errors" : "error");
    return 1;
  }

  Bytecode *iseq = compile(interp, AST);

#ifdef MXC_DEBUG
  printf(BOLD("--- compile: %s ---\n"), interp->errcnt ? "failed" : "success");
#endif

  if(interp->errcnt) {
    return 1;
  }

#ifdef MXC_DEBUG
  puts(BOLD("--- literal pool ---"));
  lpooldump(ltable);

  puts(BOLD("--- codedump ---"));
  printf("iseq len: %d\n", iseq->len);

  printf("\e[2m");
  for(size_t i = 0; i < iseq->len;) {
    codedump(iseq->code, &i, ltable);
    puts("");
  }
  puts(STR_DEFAULT);

  puts(BOLD("--- exec result ---"));
#endif

  vm_open(iseq->code, ngvars);
  int exitcode = vm_run();

  return exitcode;
}

