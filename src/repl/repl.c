#include "maxc.h"

#include "ast.h"
#include "bytecode.h"
#include "codegen.h"
#include "error.h"
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

void mxc_repl_run(const char *src, VM *vm) {
    Vector *token = lexer_run(src);
    Vector *AST = parser_run(token);
    bool isexpr = sema_analysis_repl(AST);

    if(errcnt > 0) {
        return 1;
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

    vm->vm_frame = New_Global_Frame(iseq);

    VM_run(vm);

    if(isexpr) {
        MxcObject *top = Pop();
        printf("%s\n", top->tostring(top)->str);
        DECREF(top);
    }

    vm->stack = stackptr;
}

int mxc_main_repl() {
    printf("Welcome to maxc repl mode!\n");
    printf("maxc Version %s\n", MXC_VERSION);

    filename = "<stdin>";

    int cursor = 0;
    char last_char;

    char repl_code[1024] = {0};

    VM *vm = New_VM(NULL, MAX_GLOBAL_VARS);

    for(;;) {
        errcnt = 0;

        printf(">> ");

        memset(repl_code, 0, 1024);
        cursor = 0;

        while((last_char = getchar()) != '\n') {
            if(last_char == EOF) return 0;

            repl_code[cursor++] = last_char;
        }

        if(strcmp(repl_code, ":q") == 0) {
            puts("Good Bye");
            return 0;
        }

        repl_code[cursor++] = ';';
        repl_code[cursor] = '\n';

        if(repl_code[0] == ';') continue;

        code = repl_code;

        mxc_repl_run(repl_code, vm);
    }

    return 0;
}
