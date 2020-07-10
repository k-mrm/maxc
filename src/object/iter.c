/* implementation of iterator object */
#include "object/iterobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue iterable_next(MxcIterable *iter) {
  if(Invalid_val(iter->next)) {
    return mval_invalid;
  }

  MxcValue res = SYSTEM(iter)->get(iter, iter->index);
  iter->index++;

  return res;
}

MxcValue iterable_hasnext(MxcIterable *iter) {
  if(iter->index == iter->length - 1) {
    return mval_false;
  }

  return mval_true;
}
