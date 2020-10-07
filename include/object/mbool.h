#ifndef MXC_BOOLOBJECT_H
#define MXC_BOOLOBJECT_H

#include <stdint.h>
#include "object/object.h"

static inline MxcValue bool_logor(MxcValue l, MxcValue r) {
  if(l.num || r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue bool_logand(MxcValue l, MxcValue r) {
  if(l.num && r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue bool_not(MxcValue u) {
  if(u.num)
    return mval_false;
  else
    return mval_true;
}

#endif
