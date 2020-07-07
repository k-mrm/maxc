#ifndef MXC_STRUCT_H
#define MXC_STRUCT_H

#include <stdio.h>

struct NodeVariable;
typedef struct NodeVariable NodeVariable;

typedef struct MxcStruct {
  char *name;
  NodeVariable **field;
  size_t nfield;
} MxcStruct;

MxcStruct new_cstruct(char *name, NodeVariable **f, size_t nf);

#endif
