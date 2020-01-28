#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "env.h"
#include "maxc.h"
#include "util.h"

typedef struct userfunction {
    uint16_t codesize;
    uint16_t nlvars;
    uint8_t *code;
    Varlist *var_info;
    char *name;
} userfunction;

userfunction *New_Userfunction(Bytecode *, Varlist *, char *);


#endif
