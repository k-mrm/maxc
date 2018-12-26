#include"maxc.h"

void Token::push_num(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_NUM, value});
}

void Token::push_symbol(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_SYMBOL, value});
}

void Token::push_end() {
    token_v.push_back((token_t){TOKEN_TYPE_END, ""});
}

void Token::show() {
    std::string literal;

    for(auto token: token_v) {
        if(token.type == 1)
            literal = "Number";
        else if(token.type == 2)
            literal = "Symbol";
        else if(token.type == 0)
            literal = "End";

        std::cout << "{" << literal << ": " << token.value << "}" << std::endl;
    }
}
