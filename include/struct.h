#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include "maxc.h"

struct NodeVariable;

typedef struct MxcStruct {
    char *name;
    struct NodeVariable **field;
    size_t nfield;
} MxcStruct;

MxcStruct New_MxcStruct(char *name, struct NodeVariable **f, int nf);

#endif
