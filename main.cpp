#include"maxc.h"

int main(int argc, char **argv) {
    Maxc maxc;

    if(argc != 2) {
        maxc.show_usage();
        exit(1);
    }

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

int Maxc::run(std::string src) {
    Lexer lexer;
    Token token;
    Parser parser;
    token = lexer.run(src);

    token.show();

    parser.run(token);

    return 0;
}

void Maxc::show_usage() {
    fprintf(stderr, "./maxc <Filename>\n");
}
