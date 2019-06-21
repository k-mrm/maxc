#include"maxc.h"

char *filename = nullptr;
bool iserror = false;
std::string code;

int main(int argc, char **argv) {
    Maxc maxc;

    if(argc != 2)
        maxc.show_usage();

    code = [&]() -> std::string {
        std::ifstream file_stream(argv[1]);

        if(!file_stream)
            error("file not found");

        std::istreambuf_iterator<char> file_it(file_stream), file_last;
        std::string file_src(file_it, file_last);

        return file_src;
    }();

    filename = argv[1];

    return maxc.run(code);
}

int Maxc::run(std::string src) {
    bool isdebug = false;

    Lexer lexer;

    Token token = lexer.run(src);

    if(isdebug) {
        token.show();
    }

    Parser parser = Parser(token);
    Ast_v ASTs = parser.run();

    if(isdebug) {
        for(Ast *a: ASTs) {
            parser.show(a); puts("");
        }
    }

    SemaAnalyzer sanalyzer;

    sanalyzer.run(ASTs);

    if(iserror) return 1;

    bytecode iseq;
    Constant ctable;

    BytecodeGenerator generator = BytecodeGenerator(ctable);

    generator.compile(ASTs, parser.env, iseq);

    printf("\e[2m");
    for(size_t i = 0; i < iseq.size();) {
        generator.show(iseq, i);
        puts("");
    }
    puts("");
    printf("\e[0m");

    puts("--- exec result ---");

    VM vm = VM(&ctable);

    return vm.run(iseq);
}

void Maxc::show_usage() {
    error("./maxc <Filename>");
}
