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

extern int errcnt;
extern char *filename;
extern Vector *ltable;
extern char *code;

#define MAX_GLOBAL_VARS 128

void mxc_repl_run(const char *src, Frame *frame, const char *fname) {
    Vector *token = lexer_run(src, fname);
    Vector *AST = parser_run(token);
    bool isexpr = sema_analysis_repl(AST);
    int res;

    if(errcnt > 0) {
        return;
    }

    Bytecode *iseq = compile_repl(AST);

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

    frame->code = iseq->code;
    frame->codesize = iseq->len;
    frame->pc = 0;

    res = VM_run(frame);

    if(isexpr && (res == 0)) {
        MxcObject *top = *--frame->stackptr;
        printf("%s : %s\n",
               OBJIMPL(top)->tostring(top)->str,
               OBJIMPL(top)->type_name);
    }
}

int mxc_main_repl() {
    printf("Welcome to maxc repl mode!\n");
    printf("maxc Version %s\n", MXC_VERSION);
    printf("use exit(int) or Ctrl-D to exit\n");

    filename = "<stdin>";

    int cursor = 0;
    char last_char;

    char repl_code[1024] = {0};

    Frame *frame = New_Global_Frame(NULL, MAX_GLOBAL_VARS);

    for(;;) {
        errcnt = 0;

        printf(">> ");

        memset(repl_code, 0, 1024);
        cursor = 0;

        while((last_char = getchar()) != '\n') {
            if(last_char == EOF) {
                putchar('\n');
                return 0;
            }

            repl_code[cursor++] = last_char;
        }

        repl_code[cursor++] = ';';
        repl_code[cursor] = '\n';

        if(repl_code[0] == ';') continue;

        code = repl_code;

        mxc_repl_run(repl_code, frame, filename);
    }

    return 0;
}
