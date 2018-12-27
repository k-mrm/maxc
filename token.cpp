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
        if(token.type == 1)
            literal = "Number";
        else if(token.type == 2)
            literal = "Symbol";
        else if(token.type == 0)
            literal = "End";

        std::cout << "{" << literal << ": " << token.value << "}" << std::endl;
    }
}

void Token::to_asm() {
    puts(".intel_syntax noprefix");
    puts(".global main");
    puts("main:");

    int cnt = 0;
    std::string type;

    for(token_t token: token_v) {
        if(token.type == 1 && cnt == 0) {
            std::cout << "  mov rax, " << token.value << std::endl;
        }
        else if(token.type == 1 && cnt != 0) {
            std::cout << token.value << std::endl;
        }
        else if(token.type == 2) {
            type = [&]() -> std::string {
                if(token.value == "+")
                    return "add";
                else if(token.value == "-")
                    return "sub";
            }();
            std::cout << "  " << type << " rax, ";
        }
        else if(token.type == 0) {
            std::cout << "  ret" << std::endl;
        }

        cnt++;
    }
}
