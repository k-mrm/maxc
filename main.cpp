#include"maxc.h"

int main(int argc, char **argv) {
    Maxc maxc;

    if(argc != 2) {
        maxc.show_usage();
    }

    std::string code = [&]() -> std::string {
        std::ifstream file_stream(argv[1]);

        if(!file_stream)
            error("file not found\n");

        std::istreambuf_iterator<char> file_it(file_stream), file_last;
        std::string file_src(file_it, file_last);

        return file_src;
    }();

    maxc.run(code);

    return 0;
}

int Maxc::run(std::string src) {
    bool isdebug = false;

    Lexer lexer;
    Token token;

    token = lexer.run(src);

    Parser parser;
    Ast_v ASTs = parser.run(token);

    if(isdebug) {
        token.show();
        for(Ast *a: ASTs) {
            parser.show(a);
            puts("");
        }
    }

    Program program;

    program.out(ASTs);

    return 0;
}

void Maxc::show_usage() {
    fprintf(stderr, "[error] ./maxc <Filename>\n");
    exit(1);
}
