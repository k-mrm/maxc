#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include "maxc.h"

class NodeVariable;

struct MxcStruct {
    std::string name;
    NodeVariable **field;
    size_t nfield;

    MxcStruct() {}
    MxcStruct(std::string &n, NodeVariable **f, size_t nf):
        name(n), field(f), nfield(nf) {}
};


#endif
