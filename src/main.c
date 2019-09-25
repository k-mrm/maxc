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

void show_usage() { error("./maxc <Filename>"); }

int main(int argc, char **argv) {
    if(argc != 2)
        show_usage();

    filename = argv[1];

    code = read_file(filename);
    if(!code) {
        error("%s: file not found", filename);
        return 1;
    }

    type_init();

    return Maxc_Run(code);
}

static int Maxc_Run(char *src) {
    Vector *token = lexer_run(src);

#ifdef MXC_DEBUG
    tokendump(token);
#endif

#ifdef MXC_DEBUG
    printf("\e[1m--- lex: %s ---\e[0m\n", errcnt ? "failed" : "success");
#endif

    Vector *AST = parser_run(token);

#ifdef MXC_DEBUG
    printf("\e[1m--- parse: %s ---\e[0m\n", errcnt ? "failed" : "success");
#endif

    int nglobalvars = sema_analysis(AST);

#ifdef MXC_DEBUG
    printf("\e[1m--- sema_analysis: %s ---\e[0m\n",
           errcnt ? "failed" : "success");
#endif

    if(errcnt > 0) {
        fprintf(stderr,
                "\n\e[1m%d %s generated\n\e[0m",
                errcnt,
                errcnt >= 2 ? "errors" : "error");
        return 1;
    }

    Bytecode *iseq = compile(AST);

#ifdef MXC_DEBUG
    printf("\e[1m--- compile: %s ---\e[0m\n", errcnt ? "failed" : "success");
#endif

#ifdef MXC_DEBUG
    puts("\e[1m--- codedump ---\e[0m");
    printf("iseq len: %d\n", iseq->len);

    printf("\e[2m");
    for(size_t i = 0; i < iseq->len;) {
        codedump(iseq->code, &i, ltable);
        puts("");
    }
    puts("");
    printf("\e[0m");

    puts("\e[1m--- exec result ---\e[0m");
#endif

    return VM_run(iseq, nglobalvars);
}
