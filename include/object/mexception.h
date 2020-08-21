#ifndef MXC_EXCEPTION_H
#define MXC_EXCEPTION_H

#include "object/object.h"
#include "object/strobject.h"

enum exception {
  EOUTOFRANGE,
  EZERO_DIVISION,
  EASSERT,
};

typedef struct MException MException;
struct MException {
  OBJECT_HEAD;
  enum exception e;
  MxcString *msg;
};

#endif
