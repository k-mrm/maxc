#include "maxc.h"

#define STEP()                                                                 \
    do {                                                                       \
        ++i;                                                                   \
        ++col;                                                                 \
    } while(0)
#define PREV()                                                                 \
    do {                                                                       \
        --i;                                                                   \
        --col;                                                                 \
    } while(0)

Token &Lexer::run(std::string src) {
    int line = 1;
    int col = 1;
    // std::cout << src << std::endl;

    for(unsigned int i = 0; i < src.size(); ++i, ++col) {
        if(isdigit(src[i])) {
            save(line, col);
            std::string value_num;
            bool isdot = false;

            for(; isdigit(src[i]) || src[i] == '.'; ++i, ++col) {
                value_num += src[i];

                if(src[i] == '.') {
                    if(isdot)
                        error("oi");
                    isdot = true;
                }
            }

            PREV();

            if(src[i] == '.') {
                error(". <- !!!!!!");
            }
            location_t loc = location_t(line, col);
            token.push_num(value_num, get(), loc);
        }
        else if(isalpha(src[i]) || src[i] == '_') {
            save(line, col);
            std::string ident;

            for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_';
                ++i, ++col)
                ident += src[i];

            PREV();
            location_t loc = location_t(line, col);
            token.push_ident(ident, get(), loc);
        }
        else if((src[i] == '+' && src[i + 1] == '+') ||
                (src[i] == '-' && src[i + 1] == '-') ||
                (src[i] == '&' && src[i + 1] == '&') ||
                (src[i] == '|' && src[i + 1] == '|') ||
                (src[i] == '.' && src[i + 1] == '.')) {
            location_t loc = location_t(line, col);
            std::string s;
            s = src[i];
            s += src[++i];
            ++col;

            location_t loce = location_t(line, col);
            token.push_symbol(s, loc, loce);
        }
        else if(src[i] == '-' && src[i + 1] == '>') {
            location_t loc = location_t(line, col);
            std::string allow;
            allow = src[i];
            allow += src[++i];
            ++col;

            location_t loce = location_t(line, col);

            token.push_symbol(allow, loc, loce);
        }
        else if((src[i] == '/') && (src[i + 1] == '/')) {
            for(; src[i] != '\n' && src[i] != '\0'; ++i, ++col)
                ;
            continue;
        }
        else if(src[i] == '(' || src[i] == ')' || src[i] == ',' ||
                src[i] == '{' || src[i] == '}' || src[i] == '&' ||
                src[i] == '|' || src[i] == '[' || src[i] == ']' ||
                src[i] == ':' || src[i] == '.' || src[i] == '?') {
            location_t loc = location_t(line, col);
            std::string value_symbol;

            value_symbol = src[i];
            token.push_symbol(value_symbol, loc, loc);
        }
        else if(src[i] == '=' || src[i] == '<' || src[i] == '>' ||
                src[i] == '!' || src[i] == '+' || src[i] == '-' ||
                src[i] == '*' || src[i] == '/' || src[i] == '%') {
            save(line, col);
            std::string value;
            value = src[i];
            if(src[i + 1] == '=') {
                STEP();
                value += src[i];
                if(src[i - 1] == '<' && src[i + 1] == '>') {
                    STEP();
                    value += src[i];
                }
            }

            location_t loc = location_t(line, col);

            token.push_symbol(value, get(), loc);
        }
        else if(src[i] == '\"') {
            save(line, col);
            std::string cont;
            STEP();
            for(; src[i] != '\"'; ++i, ++col) {
                cont += src[i];
                if(src[i] == '\0' || src[i] == '\n') {
                    /*
                    error(line, col, "missing charcter:`\"`");*/
                    exit(1);
                }
            }

            location_t loc = location_t(line, col);

            token.push_string(cont, get(), loc);
        }
        else if(src[i] == '\'') {
            save(line, col);
            std::string cont;
            STEP();
            cont = src[i];
            STEP();

            location_t loc = location_t(line, col);

            token.push_char(cont, get(), loc);
        }
        else if(src[i] == ';') {
            save(line, col);
            std::string comma;

            comma = src[i];
            token.push_symbol(comma, get(), get());
        }
        else if(isblank(src[i])) {
            continue;
        }
        else if(src[i] == '\n') {
            ++line;
            col = 0;
            continue;
        }
        else {
            error("invalid syntax: \" %c \"", src[i]);
            // exit(1);
        }
    }

    save(++line, col);

    token.push_end(get(), get());

    return token;
}

void Lexer::save(int l, int c) { saved_loc = location_t(l, c); }

location_t &Lexer::get() { return saved_loc; }
