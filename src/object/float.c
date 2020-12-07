/* implementation of float object */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "object/mfloat.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue float_copy(MxcValue v) {
  return v;
}

MxcValue float_eq(MxcValue l, MxcValue r) {
  if(V2F(l) == V2F(r))
    return mval_true;
  else
    return mval_false;
}

MxcValue float_neq(MxcValue l, MxcValue r) {
  if(V2F(l) != V2F(r))
    return mval_true;
  else
    return mval_false;
}

MxcValue float_lt(MxcValue l, MxcValue r) {
  if(V2F(l) < V2F(r))
    return mval_true;
  else
    return mval_false;
}

MxcValue float_lte(MxcValue l, MxcValue r) {
  if(V2F(l) <= V2F(r))
    return mval_true;
  else
    return mval_false;
}

MxcValue float_gt(MxcValue l, MxcValue r) {
  if(V2F(l) > V2F(r))
    return mval_true;
  else
    return mval_false;
}

MxcValue float_div(MxcValue l, MxcValue r) {
  if(V2F(r) == 0.0) {
    return mval_invalid;
  }

  return mval_float(V2F(l) / V2F(r));
}

MxcValue float_tostring(MxcValue val) {
  double f = V2F(val);
  size_t len = get_digit((int)f) + 10;
  char *str = malloc(sizeof(char) * len);
  sprintf(str, "%.8lf", f);

  return new_string(str, len);
} 

