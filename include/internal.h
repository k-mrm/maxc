#ifndef MXC_INTERNAL_H
#define MXC_INTERNAL_H

#ifdef NDEBUG
#define MXC_DEBUG
#endif

#define STR_DEFAULT "\e[0m"
#define BOLD(s) "\e[1m" s "\e[0m"
#define MUTED(s) "\e[2m" s "\e[0m"

#define MXC_VERSION "0.0.1"
#define INTERN_UNUSE(v) ((void)v)

void intern_abort(void);

typedef struct ReadStatus {
    char *str;
    struct {
        int eof: 1;
        int toolong: 1;
    } err;
} ReadStatus;

#endif
