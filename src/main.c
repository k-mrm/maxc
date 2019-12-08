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

char *filename = NULL;
char *code;

extern int errcnt;

static void mxc_init();
static void mxc_destructor();

extern MxcObject **stackptr;

void show_usage() { error("./maxc <Filename>"); }

int main(int argc, char **argv) {
    mxc_init();

    if(argc == 1) {
        return mxc_main_repl();
    }
    else if(argc != 2)
        show_usage();

    filename = argv[1];

    code = read_file(filename);
    if(!code) {
        error("%s: file not found", filename);
        return 1;
    }

    return mxc_main(code);
}

static void mxc_init() {
    type_init();
    setup_token();
    define_operator();
}

int mxc_main(char *src) {
    Vector *token = lexer_run(src);

#ifdef MXC_DEBUG
    tokendump(token);
#endif

#ifdef MXC_DEBUG
    printf(BOLD("--- lex: %s ---\n"), errcnt ? "failed" : "success");
#endif

    Vector *AST = parser_run(token);

#ifdef MXC_DEBUG
    printf(BOLD("--- parse: %s ---\n"), errcnt ? "failed" : "success");
#endif

    int nglobalvars = sema_analysis(AST);

#ifdef MXC_DEBUG
    printf(BOLD("--- sema_analysis: %s ---\n"),
           errcnt ? "failed" : "success");
#endif

    if(errcnt > 0) {
        fprintf(stderr,
                BOLD("\n%d %s generated\n"),
                errcnt,
                errcnt >= 2 ? "errors" : "error");
        return 1;
    }

    Bytecode *iseq = compile(AST);

#ifdef MXC_DEBUG
    printf(BOLD("--- compile: %s ---\n"), errcnt ? "failed" : "success");
#endif

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

    int exitcode = VM_run(iseq, nglobalvars);

    mxc_destructor();

    return exitcode;
}

static void mxc_destructor() {}
