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

char *filename = NULL;
char *code;
MxcArg mxc_args;

extern int errcnt;
extern MxcObject **stackptr;

static void mxc_init();
static void mxc_destructor();

void show_usage() { error("./maxc <Filename>"); }

int main(int argc, char **argv) {
    mxc_init(argc, argv);

    if(argc == 1) {
        return mxc_main_repl();
    }

    filename = argv[1];

    code = read_file(filename);
    if(!code) {
        error("%s: cannot open file", filename);
        return 1;
    }

    return mxc_main(code);
}

static void mxc_init(int argc, char **argv) {
    mxc_args = (MxcArg){argc, argv};

    setup_token();
    define_operator();
    sema_init();
}

int mxc_main(const char *src) {
    Vector *token = lexer_run(src);

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

    if(errcnt) {
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

    Frame *global_frame = New_Global_Frame(iseq, ngvars);
    int exitcode = VM_run(global_frame);

    mxc_destructor();

    return exitcode;
}

static void mxc_destructor() {}
