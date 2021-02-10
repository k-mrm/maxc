#include "object/object.h"
#include "object/num.h"
#include "object/minteger.h"
#include "object/mstr.h"
#include "mlib.h"

MxcValue num_add(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_add(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  return integer_add(x, y); 
}

MxcValue num_sub(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_sub(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  return integer_sub(x, y); 
}

MxcValue num_mul(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_mul(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  return integer_mul(x, y); 
}

MxcValue num_div(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_div(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  return integer_divrem(x, y, NULL); 
}

MxcValue num_mod(MxcValue x, MxcValue y) {
  if(isint(x) && isint(y)) {
    return int_rem(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  MxcValue r;
  integer_divrem(x, y, &r); 
  return r;
}

MxcValue num_eq(MxcValue x, MxcValue y) {
  if((isint(x) && isint(y)) || (isbool(x) && isbool(y))) {
    return int_eq(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  return integer_eq(x, y);
}

MxcValue num_noteq(MxcValue x, MxcValue y) {
  if((isint(x) && isint(y)) || (isbool(x) && isbool(y))) {
    return int_noteq(x, y);
  }
  x = isint(x) ? int_to_integer(V2I(x)) : x;
  y = isint(y) ? int_to_integer(V2I(y)) : y;

  if(istrue(integer_eq(x, y)))
    return mval_false;
  else
    return mval_true;
}

MxcValue num_neg(MxcValue x) {
  if(isint(x)) {
    return mval_int(-V2I(x));
  }

  MxcValue res = integer_copy(V2O(x));
  obig(res)->sign = SIGN_MINUS;
  return res;
}

MxcValue num2str(MxcValue x, int base) {
  if(isint(x)) {
    return int2str(x, base);
  }
  else {
    return integer2str((MInteger *)V2O(x), base);
  }
}

static MxcValue m_num2str(MxcValue *args, size_t na) {
  return num2str(args[0], V2I(args[1]));
}

void int_init() {
  MxcModule *mod = new_mxcmodule("int");

  define_cfunc(mod, "tostr", m_num2str, FTYPE(mxc_string, mxc_int, mxc_int));

  reg_gmodule(mod);
}
