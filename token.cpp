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

    for(token_t token: token_v) {
        literal = [&]() -> std::string {
            if(token.type == TOKEN_TYPE_NUM)
                return "Number";
            else if(token.type == TOKEN_TYPE_SYMBOL)
                return "Symbol";
            else if(token.type == TOKEN_TYPE_END)
                return "End";
            else {
                printf("???");
                exit(1);
            }
        }();

        std::cout << literal << "( " << token.value << " )" << std::endl;
    }
}

