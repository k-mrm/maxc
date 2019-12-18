#include "error.h"
#include "maxc.h"

extern char *filename;
extern char *code;
int errcnt = 0;

static void mxcerr_header(Location start, Location end) {
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

void error_at(const Location start, const Location end, const char *msg, ...) {
    mxcerr_header(start, end);

    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, STR_DEFAULT);

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
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


void unexpected_token(const Location start,
                      const Location end,
                      const char *unexpected, ...) {
    mxcerr_header(start, end);

    fprintf(stderr, "unexpected token: `%s`", unexpected);
    puts(STR_DEFAULT);

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");
    }

    showline(start.line, lline);

    for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
        printf(" ");
    printf("\e[31;1m");
    for(int i = 0; i < lcol; ++i)
        printf("^");
    printf(" expected: ");

    va_list expect;
    va_start(expect, unexpected);

    int ite = 0;
    for(char *t = va_arg(expect, char *); t; t = va_arg(expect, char *), ite++) {
        if(ite > 0) {
            printf(", ");
        }
        printf("`%s`", t);
    }

    printf(STR_DEFAULT);

    puts("\n");

    ++errcnt;
}

void expect_token(const Location start, const Location end, const char *token) {
    mxcerr_header(start, end);
    fprintf(stderr, "expected token: `%s`", token);
    puts(STR_DEFAULT);

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");
    }

    showline(start.line, lline);

    for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
        printf(" ");

    printf("\e[31;1m");
    for(int i = 0; i < lcol; ++i)
        printf(" ");
    printf("^");
    printf(" expected token: `%s`", token);
    printf(STR_DEFAULT);

    puts("\n");

    ++errcnt;
}

void mxc_unimplemented(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[unimplemented] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m ", filename);
    vfprintf(stderr, msg, args);
    puts("");
    va_end(args);

    errcnt++;
}

void warning(const Location start, const Location end, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr,
            "\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            start.col);
    vfprintf(stderr, msg, args);
    fprintf(stderr, STR_DEFAULT);
    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        fprintf(stderr, "\n\n");
        /*
        printf("%s", skipln(pos).c_str()); puts("");
        std::string sp = std::string(col + 1, ' ');
        printf("%s", sp.c_str()); printf("\e[34;1m^\e[0m");puts("\n");
        */
    }
    va_end(args);
}

void runtime_err(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[runtime error] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m ", filename);
    vfprintf(stderr, msg, args);
    puts("");
    va_end(args);
    exit(1);
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

    printf("\e[36;1m%d | \e[0m", line);

    int line_num = 1;

    for(size_t i = 0; i < strlen(code); ++i) {
        if(line_num == line) {
            String *lbuf = New_String();
            while(code[i] != '\n') {
                string_push(lbuf, code[i]);
                ++i;
            }

            printf("%s", lbuf->data);
            break;
        }

        if(code[i] == '\n')
            ++line_num;
    }

    puts("");

    showline(++line, --nline);
}

void mxc_assert_core(int boolean, char *message, char *file, int line) {
    if(boolean == false) {
        fprintf(stderr, "\e[31;1m[assertion failed]: \e[0m");
        fprintf(stderr, "\e[1m%s (%s:%d)\n\e[0m", message, file, line);
    }
}
