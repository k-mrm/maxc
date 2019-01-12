#include"maxc.h"

Token Lexer::run(std::string src) {
    Token token;
    //std::cout << src << std::endl;

    for(unsigned int i = 0; i < src.size(); i++) {
        if(isdigit(src[i])) {
            std::string value_num;

            for(; isdigit(src[i]);i++) {
                value_num += src[i];
            }

            --i;
            token.push_num(value_num);
        }
        else if(isalpha(src[i]) || src[i] == '_') {
            std::string ident;

            for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_'; i++)
                ident += src[i];

            --i;
            token.push_ident(ident);
        }
        else if((src[i] == '/') && (src[i + 1] == '/')) {
            for(; src[i] != '\n'; i++);
            continue;
        }
        else if(src[i] == '+' || src[i] == '-' || src[i] == '*' || src[i] == '/' ||
                src[i] == '(' || src[i] == ')' || src[i] == '%') {
            std::string value_symbol;

            value_symbol = src[i];
            token.push_symbol(value_symbol);
        }
        else if(isblank(src[i])) {
            continue;
        }
        else if(src[i] == '\n') {
            continue;
        }
        else if(src[i] == ';') {
            continue;
        }
        else {
            std::cerr << "[error] invalid syntax: " << src[i] << std::endl;
            //exit(1);
        }
        //printf("aaa%d\n", i);
    }
    token.push_end();

    return token;
}
