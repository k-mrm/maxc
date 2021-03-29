#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include "util.h"
#include "object/object.h"
#include "object/mstr.h"
#include "literalpool.h"
#include "gc.h"

extern char *filename;
extern Vector *ltable;
extern char *code;
extern size_t gc_time;
#define MAX_GLOBAL_VARS 128

void mxc_repl_run(const char *src, struct cgen *cg) {
  struct cgen *c = NULL;
  Vector *token = lexer_run(src, filename);
  struct mparser *pstate = parser_run(token);
  if(pstate->err)
    goto err;

  Vector *ast = pstate->ast;
  SemaResult sema_res = sema_analysis_repl(ast);
  if(sema_res.scope->err)
    goto err;

  c = compile_repl(ast, cg);

#ifdef MXC_DEBUG
  puts(BOLD("--- literal pool ---"));
  lpooldump(c->ltable);
  puts(BOLD("--- codedump ---"));
  printf("iseq len: %d\n", c->iseq->len);
  /*
  printf("\e[2m");
  for(size_t i = 0; i < c->iseq->len;) {
    codedump(c->iseq->code, &i, c->ltable);
    puts("");
  }
  */
  puts(STR_DEFAULT);
  puts(BOLD("--- exec result ---"));
#endif

  VM *vm = curvm();
  vm->gvars = c->gvars;
  vm->ctx->code = c->iseq->code;
  vm->ctx->pc = &c->iseq->code[0];

  int res = vm_run();

  if(sema_res.isexpr && res == 0) {
    extern enum stack_cache_state scstate;
    MxcValue top = (scstate & 1)? screg_a : scstate? screg_b : POP();
    scstate = (((int)scstate - 2) <= 0? 0 : scstate - 2);
    char *dump = ostr(mval2str(top))->str;
    printf("%s : %s\n", dump, sema_res.tyname);
  }

err:
  free(pstate);
  free(c);
}

int mxc_main_repl() {
  printf("Welcome to maxc repl mode!\n");
  printf("maxc Version %s\n", MXC_VERSION);
  printf("use exit(int) or Ctrl-D to exit\n");

  filename = "<stdin>";
  size_t cursor;
  struct cgen *c = newcgen_glb(MAX_GLOBAL_VARS);
  vm_open(NULL, NULL, MAX_GLOBAL_VARS, c->ltable, NULL);

  for(;;) {
    cursor = 0;
    printf(">> ");

    ReadStatus rs = intern_readline(1024, &cursor, ";\n", 2);
    if(rs.err.eof) {    /* repl end */
      putchar('\n');
      return 0;
    }
    if(rs.err.toolong) {
      error("Too long input");
      continue;
    }

    code = rs.str;
    mxc_repl_run(rs.str, c);

    free(rs.str);
  }

  return 0;
}
