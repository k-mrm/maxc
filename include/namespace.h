#ifndef MAXC_NAMESPACE_H
#define MAXC_NAMESPACE_H

#include "env.h"
#include "util.h"

typedef struct Namespace {
  char *name;
  Varlist *vars;
} Namespace;

extern Vector *namespace_table;

void reg_namespace(char *, Varlist *);
Varlist *search_namespace(char *);

#endif
