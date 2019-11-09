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

static int Maxc_Run(char *src);
static int mxc_repl();
static void mxc_init();
static void mxc_destructor();

extern MxcObject **stackptr;

void show_usage() { error("./maxc <Filename>"); }

int main(int argc, char **argv) {
    mxc_init();

    if(argc == 1) {
        return mxc_repl();
    }
    else if(argc != 2)
        show_usage();

    filename = argv[1];

    code = read_file(filename);
    if(!code) {
        error("%s: file not found", filename);
        return 1;
    }

    return Maxc_Run(code);
}

static void mxc_init() {
    type_init();
    setup_token();
    define_operator();
}

static int Maxc_Run(char *src) {
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
    puts("");
    printf("\e[0m");

    puts(BOLD("--- exec result ---"));
#endif

    int ret = VM_run(iseq, nglobalvars);

    mxc_destructor();

    return ret;
}

static void mxc_destructor() {}

static void print_res(MxcObject *a) {
    printf("%ld\n", ((IntObject *)a)->inum);
}

static int mxc_repl() {
    printf("Welcome to maxc!\n");
    printf("maxc Version %s\n", "0.0.1");

    for(;;) {
        char code[256] = {0};
        printf(">> ");
        scanf("%[^\n]%*c", code);

        if(strncmp(code, ":q", strlen(":q")) == 0) {
            puts("Good Bye");
            return 0;
        }

        printf("debug: %s\n", code);

        if(Maxc_Run(code) == 0) {
            print_res(Pop());
        }
    }

    return 0;
}
