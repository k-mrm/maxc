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

    maxc.run(code);

    return 0;
}

int Maxc::run(std::string src) {
    bool isdebug = false;

    Lexer lexer;
    Token token;

    token = lexer.run(src);

    if(isdebug) {
        token.show();
    }

    Parser parser;
    Ast_v ASTs = parser.run(token);

    if(isdebug) {
        for(Ast *a: ASTs) {
            parser.show(a); puts("");
        }
    }

    Program vmcode;
    VM vm;

    if(iserror)
        return 1;

    vmcode.compile(ASTs, parser.env);
    printf("\e[2m");
    vmcode.show(); puts("");
    printf("\e[0m");
    puts("--- exec result ---");
    //vm.run(vmcode.vmcodes, vmcode.lmap);

    return 0;
}

void Maxc::show_usage() {
    error("./maxc <Filename>");
}
