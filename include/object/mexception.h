#ifndef MXC_EXCEPTION_H
#define MXC_EXCEPTION_H

#include "object/object.h"
#include "object/string.h"

enum exception {
  EXC_OUTOFRANGE,
  EXC_ZERO_DIVISION,
  EXC_ASSERT,
}

typedef struct MException MException;
struct MException {
  OBJECT_HEAD;
  enum exception e;
  MxcString *msg;
};

#endif
