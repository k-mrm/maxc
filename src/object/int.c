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

void int_divrem(MxcValue l, MxcValue r, MxcValue *quo, MxcValue *rem) {
  int64_t x = V2I(l);
  int64_t y = V2I(r);
  if(y == 0) {
    mxc_raise(EXC_ZERO_DIVISION, "divide by 0");
    if(quo) *quo = mval_invalid;
    if(rem) *rem = mval_invalid;
    return;
  }

  if(x == INT64_MIN && y == -1) {
    *quo = uint_to_integer((uint64_t)INT64_MAX + 1);
    *rem = mval_int(0);
    return;
  }
  int64_t qq = x / y;
  int64_t rr = x % y;

  if(quo) *quo = mval_int(qq);
  if(rem) *rem = mval_int(rr);
}

MxcValue int2str(MxcValue val, int base) {
  bool neg = false;
  char buf[sizeof(int32_t) * CHAR_BIT + 1] = {0};
  char *end = buf + sizeof(buf);
  char *cur = end;
  int32_t num = V2I(val);
  uint32_t unum;

  if(base < 2 || 36 < base) {
    return mval_null;
  }

  if(num == 0) {
    return new_string_static("0", 1);
  }
  if(num < 0) {
    unum = (uint32_t)(-(num + 1)) + 1;
    neg = true;
  }
  else {
    unum = (uint32_t)num;
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
