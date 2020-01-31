#ifndef MXC_OPCODE_H
#define MXC_OPCODE_H

enum OPCODE {
#define OPCODE_DEF(op) OP_ ## op,
#include "opcode-def.h"
#undef OPCODE_DEF
};

#endif /* MXC_OPCODE_H */
