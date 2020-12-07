#ifndef MXC_ITEROBJECT_H
#define MXC_ITEROBJECT_H

#include <stdio.h>
#include "object/object.h"

struct MxcIterable {
  OBJECT_HEAD;
  size_t length;
  size_t index;
};

MxcValue iterable_reset(MxcIterable *);
MxcValue iterable_next(MxcIterable *);
MxcValue iterable_stopped(MxcIterable *);

#endif
