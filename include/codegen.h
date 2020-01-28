#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "ast.h"
#include "env.h"
#include "literalpool.h"

Bytecode *compile(Vector *, Vector **);
Bytecode *compile_repl(Vector *, Vector *, Vector **);

#endif
