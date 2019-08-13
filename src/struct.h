#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include "maxc.h"

class Ast;

struct MxcStruct {
    std::string name;
    Ast **field;
    size_t nfield;

    MxcStruct() {}
    MxcStruct(std::string &n, Ast **f, size_t nf):
        name(n), field(f), nfield(nf) {}
};


#endif
