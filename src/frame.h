#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include "bytecode.h"
#include "function.h"
#include "maxc.h"
#include "util.h"

typedef struct Frame {
    uint8_t *code;
    size_t codesize;
    Vector *lvars;
    size_t pc;
    size_t nlvars;
} Frame;

Frame *New_Global_Frame(Bytecode *);
Frame *New_Frame(userfunction *);
void Delete_Frame(Frame *);

#endif
