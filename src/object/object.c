#include <stdlib.h>

#include "object/object.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

const char mxc_36digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

MxcValue mval2str(MxcValue val) {
  switch(val.t) {
    case VAL_OBJ:
      return SYSTEM(val.obj)->tostring(val.obj);
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
      error("unreachable");
  }

  return mval_invalid;
}

MxcValue mval_copy(MxcValue val) {
  switch(val.t) {
    case VAL_OBJ:   return SYSTEM(V2O(val))->copy(V2O(val));
    default:        return val;
  }
}

void mgc_mark(MxcValue val) {
  switch(val.t) {
    case VAL_OBJ:   SYSTEM(val.obj)->mark(val.obj); break;
    default:        break;
  }
}

void mgc_guard(MxcValue val) {
  switch(val.t) {
    case VAL_OBJ:   SYSTEM(V2O(val))->guard(V2O(val)); break;
    default:        break;
  }
}

void mgc_unguard(MxcValue val) {
  switch(val.t) {
    case VAL_OBJ:   SYSTEM(V2O(val))->unguard(V2O(val)); break;
    default:        break;
  }
}

MxcValue new_error(const char *msg) {
  MxcError *ob = (MxcError *)Mxc_malloc(sizeof(MxcError));
  ob->errmsg = msg;

  return mval_obj(ob);
}

MxcValue new_struct(int nfield) {
  MxcIStruct *ob = (MxcIStruct *)Mxc_malloc(sizeof(MxcIStruct));
  ob->field = malloc(sizeof(MxcValue) * nfield);
  return mval_obj(ob);
}

