#include "struct.h"

MxcStruct New_MxcStruct(char *name, struct NodeVariable **f, int nf) {
    return (MxcStruct){name, f, nf};
}
