#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include <stdio.h>

#include "bytecode.h"
#include "function.h"
#include "error/runtime-err.h"

struct MxcObject;
typedef struct MxcObject MxcObject;

typedef struct Frame {
    struct Frame *prev;
    char *func_name;
    char *filename;
    uint8_t *code;
    size_t codesize;
    Varlist *lvar_info;
    MxcObject **lvars;
    MxcObject **gvars;
    size_t pc;
    size_t nlvars;
    size_t ngvars;
    size_t lineno;
    MxcObject **stackptr;
    MxcObject **stacktop;
    RuntimeErr occurred_rterr;
} Frame;

Frame *New_Global_Frame(Bytecode *, int);
Frame *New_Frame(userfunction *, Frame *);
void Delete_Frame(Frame *);

#endif
