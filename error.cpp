#include "maxc.h"

extern char *filename;
extern bool iserror;
extern std::string code;
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

    iserror = true;
    errcnt++;
}

void error(const location_t &start, const location_t &end, const char *msg,
           ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ", start.line,
            start.col);
    vfprintf(stderr, msg, args);
    puts("\e[0m");

    int lline = end.line - start.line + 1;
    int lcol = end.col - start.col + 1;

    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename);
        puts("\n");

        showline(start.line, lline);

        for(int i = 0; i < start.col + std::to_string(start.line).length() + 2;
            ++i)
            printf(" ");

        printf("\e[31;1m");
        for(int i = 0; i < lcol; ++i)
            printf("^");
        printf("\e[0m");

        puts("\n");
    }
    va_end(args);

    iserror = true;
    errcnt++;
}

void warning(const location_t &start, const location_t &end, const char *msg,
             ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ", start.line,
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

    for(unsigned int i = 0; i < code.length(); ++i) {
        if(line_num == line) {
            std::string lbuf;
            while(code[i] != '\n') {
                lbuf += code[i];
                ++i;
            }

            printf("%s", lbuf.c_str());
            break;
        }

        if(code[i] == '\n')
            ++line_num;
    }

    puts("");

    showline(++line, --nline);
}
