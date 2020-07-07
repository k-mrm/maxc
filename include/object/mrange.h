#ifndef MXC_MRANGE_H
#define MXC_MRANGE_H

#include "object/object.h"

typedef struct MRange MRange;
struct MRange {
  OBJECT_HEAD;
  MxcValue begin;
  MxcValue end;
  int excl;
};

#endif
