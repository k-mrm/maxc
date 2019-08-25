#include "maxc.h"

#include "ast.h"
#include "bytecode.h"
#include "bytecode_gen.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "token.h"
#include "vm.h"
#include "type.h"

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

    return Maxc_Run(code);
}

static int Maxc_Run(char *src) {
    Vector *token = lexer_run(src);

#ifdef MXC_DEBUG
    tokendump(token);
#endif

    type_init();

#ifdef MXC_DEBUG
    if(!errcnt)
        puts("\e[1m--- lex: success---\e[0m");
#endif

    Vector *AST = parser_run(token);

#ifdef MXC_DEBUG
    if(!errcnt)
        puts("\e[1m--- parse: success---\e[0m");
#endif

    int nglobalvars = sema_analysis(AST);

#ifdef MXC_DEBUG
    if(!errcnt)
        puts("\e[1m--- sema_analysis: success---\e[0m");
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
    printf("\e[1m--- compile: %s ---\e[0m\n",
            errcnt ? "failed" : "success");
#endif

#ifdef MXC_DEBUG
    puts("--- codedump ---");
    printf("iseq len: %d\n", iseq->len);

    printf("\e[2m");
    for(size_t i = 0; i < iseq->len;) {
        codedump(iseq->code, &i, ltable);
        puts("");
    }
    puts("");
    printf("\e[0m");

    puts("--- exec result ---");
#endif

    return VM_run(iseq, nglobalvars);
}

