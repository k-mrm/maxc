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

    token.show();

    return token;
}

void Token::push_num(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_NUM, value});
}

void Token::push_symbol(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_SYMBOL, value});
}

void Token::show() {
    for(auto token: token_v) {
        std::cout << token.type << ":" << token.value << std::endl;
    }
}
