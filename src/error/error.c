#include "error/error.h"
#include "maxc.h"

extern char *filename;
extern char *code;
int errcnt = 0;

static void mxcerr_header(SrcPos start, SrcPos end) {
    fprintf(stderr,
            "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            end.col);
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error] \e[0m");
    fprintf(stderr, "\e[1m");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\e[0m");
    fprintf(stderr, "\n");
    if(filename)
        fprintf(stderr, "\e[33;1min %s\e[0m\n", filename);
    va_end(args);

    errcnt++;
}

void error_nofile(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error] \e[0m");
    fprintf(stderr, "\e[1m");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\e[0m");
    fprintf(stderr, "\n");
    va_end(args);

    errcnt++;
}

void warn(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[94;1m[warning] \e[0m");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\e[0m");
    if(filename)
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
    fprintf(stderr, "\n");
    va_end(args);
}

void error_at(const SrcPos start, const SrcPos end, const char *msg, ...) {
    mxcerr_header(start, end);

    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, STR_DEFAULT);

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(start.filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", start.filename);
        fprintf(stderr, "\n\n");
    }

    showline(start.line, lline);

    for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
        fprintf(stderr, " ");

    fprintf(stderr, "\e[31;1m");
    for(int i = 0; i < lcol; ++i)
        fprintf(stderr, "^");
    fprintf(stderr, STR_DEFAULT);

    fprintf(stderr, "\n\n");
    va_end(args);

    errcnt++;
}

void unexpected_token(const SrcPos start,
                      const SrcPos end,
                      const char *unexpected, ...) {
    mxcerr_header(start, end);

    fprintf(stderr, "unexpected token: `%s`", unexpected);
    fprintf(stderr, STR_DEFAULT "\n");

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(start.filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", start.filename);
        fprintf(stderr, "\n\n");
    }

    showline(start.line, lline);

    for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\e[31;1m");
    for(int i = 0; i < lcol; ++i)
        fprintf(stderr, "^");
    fprintf(stderr, " expected: ");

    va_list expect;
    va_start(expect, unexpected);

    int ite = 0;
    for(char *t = va_arg(expect, char *); t; t = va_arg(expect, char *), ite++) {
        if(ite > 0) {
            fprintf(stderr, ", ");
        }
        fprintf(stderr, "`%s`", t);
    }

    fprintf(stderr, STR_DEFAULT "\n\n");

    ++errcnt;
}

void expect_token(const SrcPos start, const SrcPos end, const char *token) {
    mxcerr_header(start, end);
    fprintf(stderr, "expected token: `%s`", token);
    fprintf(stderr, STR_DEFAULT "\n");

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(start.filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", start.filename);
        fprintf(stderr, "\n\n");
    }

    showline(start.line, lline);

    for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
        fprintf(stderr, " ");

    fprintf(stderr, "\e[31;1m");
    for(int i = 0; i < lcol; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "^");
    fprintf(stderr, " expected token: `%s`", token);
    fprintf(stderr, STR_DEFAULT "\n\n");

    ++errcnt;
}

void mxc_unimplemented(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[unimplemented] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m ", filename);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);

    errcnt++;
}

void warning(const SrcPos start, const SrcPos end, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr,
            "\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            start.col);
    vfprintf(stderr, msg, args);
    fprintf(stderr, STR_DEFAULT);
    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", start.filename);
        fprintf(stderr, "\n\n");
        /*
        printf("%s", skipln(pos).c_str()); puts("");
        std::string sp = std::string(col + 1, ' ');
        printf("%s", sp.c_str()); printf("\e[34;1m^\e[0m");puts("\n");
        */
    }
    va_end(args);
}

void debug(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    printf("\e[33;1m[debug] \e[0m");
    vprintf(msg, args);
    va_end(args);
}

void showline(int line, int nline) {
    if(nline == 0)
        return;

    fprintf(stderr, "\e[36;1m%d | \e[0m", line);

    int line_num = 1;

    for(size_t i = 0; i < strlen(code); ++i) {
        if(line_num == line) {
            String *lbuf = New_String();
            while(code[i] != '\n') {
                string_push(lbuf, code[i]);
                ++i;
            }

            fprintf(stderr, "%s", lbuf->data);
            break;
        }

        if(code[i] == '\n')
            ++line_num;
    }

    fprintf(stderr, "\n");

    showline(++line, --nline);
}

void mxc_assert_core(int boolean, char *message, char *file, int line) {
    if(boolean == false) {
        fprintf(stderr, "\e[31;1m[assertion failed]: \e[0m");
        fprintf(stderr, "\e[1m%s (%s:%d)\n\e[0m", message, file, line);
    }
}
