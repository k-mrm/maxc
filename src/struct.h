#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include "maxc.h"

class NodeVariable;

struct MxcStruct {
    std::string name;
    NodeVariable **field;

    MxcStruct() {}
    MxcStruct(std::string &n, NodeVariable **f): name(n), field(f) {}
};


#endif
