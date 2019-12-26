#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include "bytecode.h"
#include "function.h"
#include "maxc.h"
#include "util.h"

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
} Frame;

Frame *New_Global_Frame(Bytecode *, int);
Frame *New_Frame(userfunction *, Frame *, MxcObject **);
void Delete_Frame(Frame *);

#endif
