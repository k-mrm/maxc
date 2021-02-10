#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include <stdbool.h>
#include "maxc.h"
#include "type.h"
#include "util.h"
#include "scope.h"

typedef struct SemaResult SemaResult;

struct SemaResult {
  Scope *scope;
  bool isexpr;
  char *tyname;
};

Scope *sema_analysis(Vector *);
SemaResult sema_analysis_repl(Vector *ast);
void sema_init(void);
Type *solvetype(Type *);

#endif
