#include"maxc.h"

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
