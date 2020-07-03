#ifndef MXC_MSTRUCT_H
#define MXC_MSTRUCT_H

#include "object/object.h"

typedef struct MxcIStruct {
  OBJECT_HEAD;
  MxcValue *field;
  uint16_t nfield;
} MStrct;

MxcValue new_struct(int);

#endif
