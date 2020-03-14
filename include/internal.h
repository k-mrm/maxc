#ifndef MXC_INTERNAL_H
#define MXC_INTERNAL_H

#include <stdio.h>
#include <stdint.h>

#ifndef NDEBUG
#define MXC_DEBUG
#endif /* !NDEBUG */

#define STR_DEFAULT "\e[0m"
#define BOLD(s) "\e[1m" s "\e[0m"
#define MUTED(s) "\e[2m" s "\e[0m"

#define MXC_VERSION "0.0.1"
#define INTERN_UNUSE(v) ((void)v)

#define log_error(...) fprintf(stderr, __VA_ARGS__)

typedef struct ReadStatus ReadStatus;

struct ReadStatus {
    char *str;
    struct {
        unsigned int eof: 1;
        unsigned int toolong: 1;
    } err;
};

ReadStatus intern_readline(size_t, size_t *, char *, size_t);
int64_t intern_scan_digit(char *, int, int *);

/* mem */
void *xmalloc(size_t);

/* die */
void intern_die(char *);
void intern_abort(void);

#endif  /* MXC_INTERNAL_H */
