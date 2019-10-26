#include "lexer.h"
#include "error.h"
#include "maxc.h"
#include "util.h"

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

static void scan(Vector *, char *);

Vector *lexer_run(char *src) {
    Vector *tokens = New_Vector();
    setup_token();

    scan(tokens, src);

    return tokens;
}

static void scan(Vector *tk, char *src) {
    int line = 1;
    int col = 1;

    for(unsigned int i = 0; i < strlen(src); ++i, ++col) {
        if(isdigit(src[i])) {
            Location start = New_Location(line, col);
            String *value_num = New_String();
            bool isdot = false;

            for(; isdigit(src[i]) || src[i] == '.'; ++i, ++col) {
                string_push(value_num, src[i]);

                if(src[i] == '.') {
                    if(isdot)
                        error("oi");
                    isdot = true;
                }
            }

            PREV();

            if(src[i] == '.') {
                PREV();
                string_pop(value_num);
            }
            Location end = New_Location(line, col);
            token_push_num(tk, value_num, start, end);
        }
        else if(isalpha(src[i]) || src[i] == '_') {
            Location start = New_Location(line, col);
            String *ident = New_String();

            for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_';
                ++i, ++col)
                string_push(ident, src[i]);

            PREV();
            Location end = New_Location(line, col);
            token_push_ident(tk, ident, start, end);
        }
        else if((src[i] == '+' && src[i + 1] == '+') ||
                (src[i] == '-' && src[i + 1] == '-') ||
                (src[i] == '-' && src[i + 1] == '>') ||
                (src[i] == '&' && src[i + 1] == '&') ||
                (src[i] == '|' && src[i + 1] == '|') ||
                (src[i] == '.' && src[i + 1] == '.') ||
                (src[i] == '>' && src[i + 1] == '>') ||
                (src[i] == '=' && src[i + 1] == '>') ||
                (src[i] == '<' && src[i + 1] == '<')) {
            Location s = New_Location(line, col);

            enum TKIND kind = tk_char2(src[i], src[i + 1]);
            STEP();

            Location e = New_Location(line, col);
            token_push_symbol(tk, kind, 2, s, e);
        }
        else if((src[i] == '/') && (src[i + 1] == '/')) {
            for(; src[i] != '\n' && src[i] != '\0'; ++i, ++col);

            PREV();
            continue;
        }
        else if(src[i] == '(' || src[i] == ')' || src[i] == ',' ||
                src[i] == '{' || src[i] == '}' || src[i] == '&' ||
                src[i] == '|' || src[i] == '[' || src[i] == ']' ||
                src[i] == ':' || src[i] == '.' || src[i] == '?' ||
                src[i] == ';') {
            Location loc = New_Location(line, col);

            enum TKIND kind = tk_char1(src[i]);
            token_push_symbol(tk, kind, 1, loc, loc);
        }
        else if(src[i] == '=' || src[i] == '<' || src[i] == '>' ||
                src[i] == '!' || src[i] == '+' || src[i] == '-' ||
                src[i] == '*' || src[i] == '/' || src[i] == '%') {
            Location s = New_Location(line, col);
            Location e;

            enum TKIND kind;
            if(src[i + 1] == '=') {
                kind = tk_char2(src[i], src[i + 1]);
                STEP();
                e = New_Location(line, col);
                token_push_symbol(tk, kind, 2, s, e);
            }
            else {
                kind = tk_char1(src[i]);
                e = New_Location(line, col);
                token_push_symbol(tk, kind, 1, s, e);
            }
        }
        else if(src[i] == '\"') {
            Location s = New_Location(line, col);
            String *cont = New_String();
            STEP();
            for(; src[i] != '\"'; ++i, ++col) {
                string_push(cont, src[i]);
                if(src[i] == '\0') {
                    /*
                    error(line, col, "missing charcter:`\"`");*/
                    exit(1);
                }
            }

            Location e = New_Location(line, col);

            token_push_string(tk, cont, s, e);
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

    Location eof = New_Location(++line, col);

    token_push_end(tk, eof, eof);
}
