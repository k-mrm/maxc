#ifndef MXC_OPCODE_H
#define MXC_OPCODE_H

#include "maxc.h"

enum OPCODE {
#ifdef DIRECT_THREADED
# define OPCODE_DEF(op) OP_ ## op,
# include "opcode-def.h"
# undef OPCODE_DEF
#else
# define OPCODE_DEF(op)  \
    OP_ ## op,  /* base(dummy) */ \
    OP_ ## op ## _SCXX,  /* SCXX base * 6 + 1 */\
    OP_ ## op ## _SCAX,  \
    OP_ ## op ## _SCBX,  \
    OP_ ## op ## _SCBA,  \
    OP_ ## op ## _SCAB,
# include "opcode-def.h"
# undef OPCODE_DEF
#endif
};

#endif /* MXC_OPCODE_H */
