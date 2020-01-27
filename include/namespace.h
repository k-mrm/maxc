#ifndef MAXC_NAMESPACE_H
#define MAXC_NAMESPACE_H

#include "maxc.h"
#include "env.h"
#include "util.h"

typedef struct Namespace {
    char *name;
    Varlist *vars;
} Namespace;

extern Vector *namespace_table;

void Register_Namespace(char *, Varlist *);
Varlist *Search_Namespace(char *);

#endif
