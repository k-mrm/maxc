#ifndef MAXC_NAMESPACE_H
#define MAXC_NAMESPACE_H

#include "util.h"

typedef struct Namespace {
  char *name;
  Vector *vars;
} Namespace;

extern Vector *namespace_table;

void reg_namespace(char *, Vector *);
Vector *search_namespace(char *);

#endif
