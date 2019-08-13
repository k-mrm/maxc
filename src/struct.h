#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include "maxc.h"

class NodeVariable;

struct MxcStruct {
    const char *name;
    NodeVariable **field;

    MxcStruct() {}
    MxcStruct(const char *n, NodeVariable **f): name(n), field(f) {}
};


#endif
