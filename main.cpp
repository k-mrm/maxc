#include"maxc.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        show_usage();
        exit(1);
    }

    Maxc maxc;

    std::string code = [&]() -> std::string {
        std::ifstream file_stream(argv[1]);

        if(!file_stream) {
            puts("[error]: file not found");
            exit(1);
        }

        std::istreambuf_iterator<char> file_it(file_stream), file_last;
        std::string file_src(file_it, file_last);

        return file_src;
    }();

    maxc.run(code);

    return 0;
}

void show_usage() {
    fprintf(stderr, "./a.out <Filename>\n");
}

int Maxc::run(std::string src) {
    Lexer lexer;
    Token token;
    token = lexer.lex(src);

    token.show();

    //token.to_asm();

    return 0;
}
