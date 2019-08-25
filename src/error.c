#include "error.h"
#include "maxc.h"

extern char *filename;
extern char *code;
int errcnt = 0;

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m ", filename);
    vfprintf(stderr, msg, args);
    puts("");
    va_end(args);

    errcnt++;
}

void error_at(const Location start,
              const Location end,
              const char *msg,
              ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr,
            "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            start.col);
    vfprintf(stderr, msg, args);
    puts("\e[0m");

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");

        showline(start.line, lline);

        for(size_t i = 0;
            i < start.col + get_digit(start.line) + 2;
            ++i)
            printf(" ");

        printf("\e[31;1m");
        for(int i = 0; i < lcol; ++i)
            printf("^");
        printf("\e[0m");

        puts("\n");
    }
    va_end(args);

    errcnt++;
}

void expect_token(const Location start,
                  const Location end,
                  const char *token) {
    fprintf(stderr,
            "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            end.col);
    fprintf(stderr, "expected token: `%s`", token);
    puts("\e[0m");

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");

        showline(start.line, lline);

        for(size_t i = 0;
            i < start.col + get_digit(start.line) + 2;
            ++i)
            printf(" ");

        for(int i = 0; i < lcol; ++i)
            printf(" ");
        printf("\e[31;1m");
        printf("^");
        printf(" expected token: `%s`", token);
        printf("\e[0m");

        puts("\n");
    }

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

void warning(const Location start,
             const Location end,
             const char *msg,
             ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr,
            "\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ",
            start.line,
            start.col);
    vfprintf(stderr, msg, args);
    puts("\e[0m");
    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");
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
