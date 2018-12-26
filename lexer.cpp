#include"maxc.h"

Token Lexer::lex(std::string src) {
    Token token;
    std::cout << src << std::endl;

    for(int i = 0; i < src.size() - 1; i++) {
        if(isdigit(src[i])) {
            std::string value_num;

            for(; isdigit(src[i]);i++) {
                value_num += src[i];
            }

            --i;
            token.push_num(value_num);
        }
        else if(src[i] == '+' || src[i] == '-') {
            std::string value_symbol;

            value_symbol += src[i];
            token.push_symbol(value_symbol);
        }
        else if(isblank(src[i])) {
            continue;
        }
        else {
            std::cout << "[error] invalid syntax: " << src[i] << std::endl;
            exit(1);
        }
    }
    token.push_end();

    token.show();

    return token;
}
