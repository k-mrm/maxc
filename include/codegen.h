#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "bytecode.h"

Bytecode *compile(Vector *);
Bytecode *compile_repl(Vector *, Vector *);

#endif
