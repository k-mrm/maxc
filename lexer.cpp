#include"maxc.h"

Token Lexer::run(std::string src) {
    Token token;
    int line = 1;
    int col = 1;
    //std::cout << src << std::endl;

    for(unsigned int i = 0; i < src.size(); ++i, ++col) {
        if(isdigit(src[i])) {
            std::string value_num;

            for(; isdigit(src[i]); ++i, ++col) {
                value_num += src[i];
            }

            --i; --col;
            token.push_num(value_num, line, col);
        }
        else if(isalpha(src[i]) || src[i] == '_') {
            std::string ident;

            for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_'; ++i, ++col)
                ident += src[i];

            --i; --col;
            token.push_ident(ident, line, col);
        }
        else if((src[i] == '+' && src[i + 1] == '+') || (src[i] == '-' && src[i + 1] == '-') ||
                (src[i] == '&' && src[i + 1] == '&') || (src[i] == '|' && src[i + 1] == '|')) {
            std::string una;
            una = src[i];
            una += src[++i];
            ++col;

            token.push_symbol(una, line, col);
        }
        else if(src[i] == '-' && src[i + 1] == '>') {
            std::string allow;
            allow = src[i]; allow += src[++i]; ++col;

            token.push_symbol(allow, line, col);
        }
        else if((src[i] == '/') && (src[i + 1] == '/')) {
            for(; src[i] != '\n'; ++i, ++col);
            continue;
        }
        else if(src[i] == '(' || src[i] == ')' || src[i] == ',' || src[i] == '{' ||
                src[i] == '}' || src[i] == '&' || src[i] == '|' || src[i] == '[' ||
                src[i] == ']' || src[i] == ':') {
            std::string value_symbol;

            value_symbol = src[i];
            token.push_symbol(value_symbol, line, col);
        }
        else if(src[i] == '=' || src[i] == '<' || src[i] == '>' || src[i] == '!' ||
                src[i] == '+' || src[i] == '-' || src[i] == '*' || src[i] == '/' ||
                src[i] == '%') {
            std::string value;
            value = src[i];
            if(src[i + 1] == '=') {
                ++i; ++col;
                value += src[i];
                if(src[i - 1] == '<' && src[i + 1] == '>') {
                    ++i; ++col;
                    value += src[i];
                }
            }

            token.push_symbol(value, line, col);
        }
        else if(src[i] == '\"') {
            std::string cont; ++i; ++col;
            for(; src[i] != '\"'; ++i, ++col) {
                cont += src[i];
            }

            token.push_string(cont, line, col);
        }
        else if(src[i] == '\'') {
            std::string cont; ++i; ++col;
            cont = src[i];
            ++i; ++col;

            token.push_char(cont, line, col);
        }
        else if(src[i] == ';') {
            std::string comma;

            comma = src[i];
            token.push_symbol(comma, line, col);
        }
        else if(isblank(src[i])) {
            continue;
        }
        else if(src[i] == '\n') {
            line++; col = 1;
            continue;
        }
        else {
            error("invalid syntax: \" %c \"", src[i]);
            //exit(1);
        }
    }
    token.push_end(--line, --col);

    return token;
}
