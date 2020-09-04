#include "object/num.h"

MxcValue num_add(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_add(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  return integer_add(x, y); 
}

MxcValue num_sub(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_sub(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  return integer_sub(x, y); 
}

MxcValue num_mul(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_mul(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  return integer_mul(x, y); 
}

MxcValue num_div(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_div(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  return integer_divrem(x, y, NULL); 
}

MxcValue num_mod(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_rem(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  MxcValue r;
  integer_divrem(x, y, &r); 
  return r;
}

MxcValue num_eq(MxcValue x, MxcValue y) {
  if((isint(x) && isint(y)) || (isbool(x) && isbool(y))) {
    return int_eq(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  return integer_eq(x, y);
}

MxcValue num_noteq(MxcValue x, MxcValue y) {
  if((isint(x) && isint(y)) || (isbool(x) && isbool(y))) {
    return int_noteq(x, y);
  }
  x = isint(x) ? int_to_integer(x.num) : x;
  y = isint(y) ? int_to_integer(y.num) : y;

  if(integer_eq(x, y).num)
    return mval_false;
  else
    return mval_true;
}

MxcValue num_neg(MxcValue x) {
  if(isint(x)) {
    return mval_int(-(x.num));
  }

  MxcValue res = integer_copy(x.obj);
  obig(res)->sign = SIGN_MINUS;
  return res;
}
