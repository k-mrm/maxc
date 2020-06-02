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
#include "module.h"

void mxc_interp_open(int argc, char **argv) {
  MInterp *interp = our_interp();
  interp->argc = argc;
  interp->argv = argv;
  interp->cur_frame = NULL;
  interp->is_vm_running = false;
  interp->errcnt = 0;
  load_default_module();
  sema_init();
}

static void mxc_destructor() {
  MInterp *interp = our_interp();
}

int mxc_main(const char *src, const char *fname) {
  MInterp *interp = our_interp();
  Vector *token = lexer_run(src, fname);

#ifdef MXC_DEBUG
  tokendump(token);
  printf(BOLD("--- lex: %s ---\n"), errcnt ? "failed" : "success");
#endif

  Vector *AST = parser_run(token);

#ifdef MXC_DEBUG
  printf(BOLD("--- parse: %s ---\n"), errcnt ? "failed" : "success");
#endif

  int ngvars = sema_analysis(AST);

#ifdef MXC_DEBUG
  printf(BOLD("--- sema_analysis: %s ---\n"),
      errcnt ? "failed" : "success");
#endif

  if(interp->errcnt) {
    fprintf(stderr, BOLD("\n%d %s generated\n"),
        interp->errcnt, interp->errcnt >= 2 ? "errors" : "error");
    return 1;
  }

  Bytecode *iseq = compile(AST);

#ifdef MXC_DEBUG
  printf(BOLD("--- compile: %s ---\n"), errcnt ? "failed" : "success");
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

  Frame *global_frame = new_global_frame(iseq, ngvars);
  int exitcode = VM_run(global_frame);

  mxc_destructor();

  return exitcode;
}

