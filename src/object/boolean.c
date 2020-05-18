/* implementation of boolean object */

#include "object/boolobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue bool_logor(MxcValue l, MxcValue r) {
  if(l.num || r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue bool_logand(MxcValue l, MxcValue r) {
  if(l.num && r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue bool_not(MxcValue u) {
  if(u.num)
    return mval_false;
  else
    return mval_true;
}

