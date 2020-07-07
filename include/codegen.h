#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "bytecode.h"
#include "maxc.h"

Bytecode *compile(MInterp *, Vector *);
Bytecode *compile_repl(MInterp *, Vector *, Vector *);

#endif
