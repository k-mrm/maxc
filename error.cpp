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
    vfprintf(stderr, msg, args); puts("");
    va_end(args);

    iserror = true;
    errcnt++;
}

void error(int pos, int col, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ", pos, col);
    vfprintf(stderr, msg, args); puts("\e[0m");
    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename); puts("\n");
        printf("%s", skipln(pos).c_str()); puts("");
        std::string sp = std::string(col - 2, ' ');
        printf("%s", sp.c_str()); printf("^");puts("\n");
    }
    va_end(args);

    iserror = true;
    errcnt++;
}

void warning(int pos, int col,  const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ", pos, col);
    vfprintf(stderr, msg, args); puts("\e[0m");
    if(filename) {
        fprintf(stderr, "\e[33;1min %s\e[0m ", filename); puts("\n");
        printf("%s", skipln(pos).c_str()); puts("");
        std::string sp = std::string(col - 2, ' ');
        printf("%s", sp.c_str()); printf("^");puts("\n");
    }
    va_end(args);
}

void runtime_err(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[runtime-error] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m ", filename);
    vfprintf(stderr, msg, args); puts("");
    va_end(args);
    exit(1);
}

void debug(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    printf("#\e[33;1m[debug] \e[0m");
    vprintf(msg, args);
    va_end(args);
}

std::string skipln(int n) {
    int line = 1;
    for(unsigned int i = 0; i < code.length(); i++) {
        if(line == n) {
            std::string lbuf;
            while(code[i] != '\n') {
                lbuf += code[i]; i++;
            }
            return lbuf;
        }
        if(code[i] == '\n')
            line++;
    }
    return "?";
}
