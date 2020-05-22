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
#include "object/strobject.h"
#include "literalpool.h"
#include "gc.h"

extern int errcnt;
extern char *filename;
extern Vector *ltable;
extern char *code;
extern size_t gc_time;
#define MAX_GLOBAL_VARS 128

void mxc_repl_run(const char *src,
    Frame *frame,
    const char *fname,
    Vector *lpool) {
  Vector *token = lexer_run(src, fname);
  Vector *AST = parser_run(token);
  SemaResult sema_res = sema_analysis_repl(AST);
  int res;

  if(errcnt > 0) {
    return;
  }

  Bytecode *iseq = compile_repl(AST, lpool);

#ifdef MXC_DEBUG
  puts(BOLD("--- literal pool ---"));
  lpooldump(lpool);
  puts(BOLD("--- codedump ---"));
  printf("iseq len: %d\n", iseq->len);
  printf("\e[2m");
  for(size_t i = 0; i < iseq->len;) {
    codedump(iseq->code, &i, lpool);
    puts("");
  }
  puts(STR_DEFAULT);
  puts(BOLD("--- exec result ---"));
#endif

  frame->code = iseq->code;
  frame->codesize = iseq->len;
  frame->pc = 0;

  res = VM_run(frame);

  if(sema_res.isexpr && (res == 0)) {
    MxcValue top = Pop();
    char *dump = ostr(mval2str(top))->str;
    printf("%s : %s\n",
        dump,
        sema_res.tyname);
  }
}

int mxc_main_repl() {
  printf("Welcome to maxc repl mode!\n");
  printf("maxc Version %s\n", MXC_VERSION);
  printf("use exit(int) or Ctrl-D to exit\n");

  filename = "<stdin>";
  size_t cursor;
  Frame *frame = new_global_frame(NULL, MAX_GLOBAL_VARS);
  Vector *litpool = new_vector();

  for(;;) {
    errcnt = 0;
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

    if(rs.str[0] == ';') continue;
    code = rs.str;
    mxc_repl_run(rs.str, frame, filename, litpool);

    free(rs.str);
  }

  return 0;
}
