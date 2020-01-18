#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include "bytecode.h"
#include "function.h"
#include "maxc.h"
#include "util.h"
#include "error/runtime-err.h"

struct MxcObject;
typedef struct MxcObject MxcObject;

typedef struct Frame {
    struct Frame *prev;
    uint8_t *code;
    size_t codesize;
    MxcObject **lvars;
    MxcObject **gvars;
    size_t pc;
    size_t nlvars;
    MxcObject **stackptr;
    RuntimeErr occurred_rterr;
} Frame;

Frame *New_Global_Frame(Bytecode *, int);
Frame *New_Frame(userfunction *, Frame *);
void Delete_Frame(Frame *);

#endif
