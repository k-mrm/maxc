#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include <stdbool.h>

#include "util.h"

typedef struct SemaResult SemaResult;

struct SemaResult {
    bool isexpr;
    char *tyname;
};

int sema_analysis(Vector *);
SemaResult sema_analysis_repl(Vector *);
void setup_bltin(void);
void sema_init(void);

#endif
