#include "lexer.h"
#include "error/error.h"
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

static void scan(Vector *, const char *, const char *);

Vector *lexer_run(const char *src, const char *fname) {
    Vector *tokens = New_Vector();
    scan(tokens, src, fname);

    return tokens;
}

static void scan(Vector *tk, const char *src, const char *fname) {
    int line = 1;
    int col = 1;
    size_t src_len = strlen(src);

    for(size_t i = 0; i < src_len; ++i, ++col) {
        if(isdigit(src[i])) {
            SrcPos start = New_SrcPos(fname, line, col);
            String *value_num = New_String();
            bool isdot = false;

            for(; isdigit(src[i]) || src[i] == '.'; ++i, ++col) {
                string_push(value_num, src[i]);

                if(src[i] == '.') {
                    if(isdot) {
                        break;
                    }
                    isdot = true;
                }
            }

            PREV();

            if(src[i] == '.') {
                /*
                 *  30.fibo()
                 *    ^
                 */
                PREV();
                string_pop(value_num);
            }
            SrcPos end = New_SrcPos(fname, line, col);
            token_push_num(tk, value_num, start, end);
        }
        else if(isalpha(src[i]) || src[i] == '_') {
            SrcPos start = New_SrcPos(fname, line, col);
            String *ident = New_String();

            for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_';
                ++i, ++col)
                string_push(ident, src[i]);

            PREV();
            SrcPos end = New_SrcPos(fname, line, col);
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
            SrcPos s = New_SrcPos(fname, line, col);

            enum TKIND kind = tk_char2(src[i], src[i + 1]);
            STEP();

            SrcPos e = New_SrcPos(fname, line, col);
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
            SrcPos loc = New_SrcPos(fname, line, col);

            enum TKIND kind = tk_char1(src[i]);
            token_push_symbol(tk, kind, 1, loc, loc);
        }
        else if(src[i] == '=' || src[i] == '<' || src[i] == '>' ||
                src[i] == '!' || src[i] == '+' || src[i] == '-' ||
                src[i] == '*' || src[i] == '/' || src[i] == '%') {
            SrcPos s = New_SrcPos(fname, line, col);
            SrcPos e;

            enum TKIND kind;
            if(src[i + 1] == '=') {
                kind = tk_char2(src[i], src[i + 1]);
                STEP();
                e = New_SrcPos(fname, line, col);
                token_push_symbol(tk, kind, 2, s, e);
            }
            else {
                kind = tk_char1(src[i]);
                e = New_SrcPos(fname, line, col);
                token_push_symbol(tk, kind, 1, s, e);
            }
        }
        else if(src[i] == '\"') {
            SrcPos s = New_SrcPos(fname, line, col);
            String *cont = New_String();
            STEP();
            for(; src[i] != '\"'; ++i, ++col) {
                string_push(cont, src[i]);
                if(src[i] == '\0') {
                    error("missing charcter:`\"`");
                    exit(1);
                }
            }

            SrcPos e = New_SrcPos(fname, line, col);

            token_push_string(tk, cont, s, e);
        }
        else if(src[i] == '`') {
            SrcPos s = New_SrcPos(fname, line, col);
            String *cont = New_String();
            STEP();

            for(; src[i] != '`'; ++i, ++col) {
                string_push(cont, src[i]);

                if(src[i] == '\0') {
                    error("missing charcter:'`'");
                    exit(1);
                }
            }

            SrcPos e = New_SrcPos(fname, line, col);

            token_push_backquote_lit(tk, cont, s, e);
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

    SrcPos eof = New_SrcPos(fname, ++line, col);
    token_push_end(tk, eof, eof);
}
