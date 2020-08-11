/* implementation of iterator object */
#include "object/iterobject.h"
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
  MxcValue res = SYSTEM(iter)->get(iter, iter->index);
  iter->index++;

  return res;
}
