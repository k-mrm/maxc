#include <stdlib.h>
#include "object/object.h"
#include "object/system.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

const char mxc_36digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

MxcValue mval2str(MxcValue val) {
  switch(mval_type(val)) {
    case VAL_OBJ:
      return SYSTEM(V2O(val))->tostring(V2O(val));
    case VAL_INT:
      return int_tostring(val);
    case VAL_FLO:
      return float_tostring(val);
    case VAL_TRUE:
      return new_string_static("true", 4);
    case VAL_FALSE:
      return new_string_static("false", 5);
    case VAL_NULL:
      return new_string_static("null", 4);
    default:
      panic("unreachable");
  }

  return mval_invalid;
}

MxcValue mval_copy(MxcValue val) {
  switch(mval_type(val)) {
    case VAL_OBJ:   return SYSTEM(V2O(val))->copy(V2O(val));
    default:        return val;
  }
}

void mgc_mark(MxcValue val) {
  switch(mval_type(val)) {
    case VAL_OBJ:   SYSTEM(V2O(val))->mark(V2O(val)); break;
    default:        break;
  }
}

void mgc_guard(MxcValue val) {
  switch(mval_type(val)) {
    case VAL_OBJ:   SYSTEM(V2O(val))->guard(V2O(val)); break;
    default:        break;
  }
}

void mgc_unguard(MxcValue val) {
  switch(mval_type(val)) {
    case VAL_OBJ:   SYSTEM(V2O(val))->unguard(V2O(val)); break;
    default:        break;
  }
}

uint32_t obj_hash32(MxcValue v) {
  return (uint32_t)V2I(v);
}
