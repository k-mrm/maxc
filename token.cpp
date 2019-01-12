#include"maxc.h"

void Token::push_num(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_NUM, value});
}

void Token::push_symbol(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_SYMBOL, value});
}

void Token::push_ident(std::string value) {
    token_v.push_back((token_t){TOKEN_TYPE_IDENTIFER, value});
}

void Token::push_end() {
    token_v.push_back((token_t){TOKEN_TYPE_END, ""});
}

token_t Token::get() {
    return token_v[pos++];
}

bool Token::is_value(std::string tk) {
    return token_v[pos].value == tk;
}

bool Token::is_type(Token_type ty) {
    return token_v[pos].type == ty;
}

void Token::step() {
    pos++;
}

bool Token::skip(std::string val) {
    if(token_v[pos].value == val) {
        pos++;
        return true;
    }
    else
        return false;
}

bool Token::step_to(std::string val) {
   return token_v[pos++].value == val;
}

void Token::show() {
    std::string literal;

    for(token_t token: token_v) {
        literal = [&]() -> std::string {
            if(token.type == TOKEN_TYPE_NUM)
                return "Number";
            else if(token.type == TOKEN_TYPE_SYMBOL)
                return "Symbol";
            else if(token.type == TOKEN_TYPE_IDENTIFER)
                return "Identifer";
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

