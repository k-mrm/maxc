/* implementation of integer object */
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include "object/mint.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue int_lt(MxcValue l, MxcValue r) {
  if(l.num < r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue int_lte(MxcValue l, MxcValue r) {
  if(l.num <= r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue int_gt(MxcValue l, MxcValue r) {
  if(l.num > r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue int_gte(MxcValue l, MxcValue r) {
  if(l.num >= r.num)
    return mval_true;
  else
    return mval_false;
}

MxcValue int2str(MxcValue val, int base) {
  bool neg = false;
  char buf[sizeof(int64_t) * CHAR_BIT + 1];
  char *end = buf + sizeof(buf);
  char *cur = end;
  int64_t num = val.num;
  uint64_t unum;

  if(base < 2 || 36 < base) {
    return mval_invalid;
  }

  if(num == 0) {
    return new_string_static("0", 1);
  }
  if(num < 0) {
    unum = (uint64_t)(-(num + 1)) + 1;
    neg = true;
  }
  else {
    unum = (uint64_t)num;
  }

  do {
    *--cur = mxc_36digits[unum % base];
  } while(unum /= base);
  if(neg) {
    *--cur = '-';
  }

  return new_string_copy(cur, end - cur);
}

MxcValue int_tostring(MxcValue val) {
  return int2str(val, 10);
}
