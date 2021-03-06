/* implementation of iterator object */
#include "object/miter.h"
#include "object/system.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue iterable_reset(MxcIterable *iter) {
  iter->index = 0;
  return mval_obj(iter);
}

MxcValue iterable_stopped(MxcIterable *iter) {
  return iter->index == iter->length? mval_true : mval_false;
}

MxcValue iterable_next(MxcIterable *iter) {
  if(iter->length <= iter->index)
    return mval_invalid;

  MxcValue res = SYSTEM(iter)->get(iter, mval_int(iter->index));
  iter->index++;
  return res;
}
