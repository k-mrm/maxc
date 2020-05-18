#include "struct.h"

MxcStruct New_MxcStruct(char *name, struct NodeVariable **f, size_t nf) {
  return (MxcStruct){name, f, nf};
}
