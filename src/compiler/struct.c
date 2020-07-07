#include "struct.h"

MxcStruct new_cstruct(char *name, struct NodeVariable **f, size_t nf) {
  return (MxcStruct){name, f, nf};
}
