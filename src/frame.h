#ifndef MXC_FRAME_H
#define MXC_FRAME_H

struct MxcObject;

#include "bytecode.h"
#include "function.h"
#include "maxc.h"
#include "util.h"

typedef struct Frame {
    struct Frame *prev;
    //frame
    uint8_t *code;
    //bytecode
    size_t codesize;
    //bytecode length
    struct MxcObject **lvars;
    //array to store local variables
    size_t pc;
    //program counter
    size_t nlvars;
    //number of local variables
} Frame;

Frame *New_Global_Frame(Bytecode *);
Frame *New_Frame(userfunction *, Frame *);
void Delete_Frame(Frame *);

#endif
