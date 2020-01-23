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

typedef struct ReadStatus ReadStatus;

struct ReadStatus {
    char *str;
    struct {
        unsigned int eof: 1;
        unsigned int toolong: 1;
    } err;
};

ReadStatus intern_readline(size_t, size_t *, char *, size_t);

/* mem */
void *xmalloc(size_t);

/* die */
void intern_die(char *);
void intern_abort(void);

#endif  /* MXC_INTERNAL_H */
