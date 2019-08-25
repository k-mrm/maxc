#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include "maxc.h"
#include "bytecode.h"
#include "util.h"
#include "function.h"

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
