#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include <stdbool.h>
#include "maxc.h"
#include "type.h"
#include "util.h"

typedef struct SemaResult SemaResult;

struct SemaResult {
  bool isexpr;
  char *tyname;
};

int sema_analysis(Vector *);
SemaResult sema_analysis_repl(Vector *);
void sema_init(MInterp *);
Type *solvetype(Type *);

#endif
