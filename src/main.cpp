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

char *filename = nullptr;
std::string code;

extern int errcnt;

int main(int argc, char **argv) {
    Maxc maxc;

    if(argc != 2)
        maxc.show_usage();

    code = [&]() -> std::string {
        std::ifstream file_stream(argv[1]);

        if(!file_stream) {
            error("file not found");
        }

        std::istreambuf_iterator<char> file_it(file_stream), file_last;
        std::string file_src(file_it, file_last);

        return file_src;
    }();

    filename = argv[1];

    return maxc.run(code);
}

int Maxc::run(std::string src) {
    Lexer lexer;

    Token token = lexer.run(src);

#ifdef MXC_DEBUG
    token.show();
#endif

    Parser parser = Parser(token);
    Ast_v ASTs = parser.run();

    SemaAnalyzer sanalyzer;

    sanalyzer.run(ASTs);

    if(errcnt > 0) {
        fprintf(stderr,
                "\n\e[1m%d %s generated\n\e[0m",
                errcnt,
                errcnt >= 2 ? "errors" : "error");
        return 1;
    }

    bytecode iseq;
    LiteralPool ltable;

    BytecodeGenerator generator = BytecodeGenerator(ltable);

    generator.compile(ASTs, iseq);

    uint8_t *bcode = &iseq[0];
    size_t codesize = iseq.size();

#ifdef MXC_DEBUG
    printf("\e[2m");
    for(size_t i = 0; i < codesize;) {
        codedump(bcode, i, ltable);
        puts("");
    }
    puts("");
    printf("\e[0m");

    puts("--- exec result ---");
#endif

    VM vm = VM(ltable, sanalyzer.ngvar);

    return vm.run(bcode, codesize);
}

void Maxc::show_usage() { error("./maxc <Filename>"); }
