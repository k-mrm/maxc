#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "env.h"
#include "maxc.h"

typedef struct userfunction {
    uint16_t codesize;
    uint16_t nlvars;
    uint8_t *code;
} userfunction;

userfunction *New_Userfunction(Bytecode *, Varlist *);


#endif
