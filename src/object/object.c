#include <stdlib.h>

#include "object/object.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue mval2str(MxcValue val) {
    switch(val.t) {
    case VAL_OBJ:
        return OBJIMPL(val.obj)->tostring(val.obj);
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
    case VAL_OBJ:   return OBJIMPL(optr(val))->copy(optr(val));
    default:        return val;
    }
}

void mgc_mark(MxcValue val) {
    switch(val.t) {
    case VAL_OBJ:   OBJIMPL(val.obj)->mark(val.obj); break;
    default:        break;
    }
}

void mgc_guard(MxcValue val) {
    switch(val.t) {
    case VAL_OBJ:   OBJIMPL(optr(val))->guard(optr(val)); break;
    default:        break;
    }
}

void mgc_unguard(MxcValue val) {
    switch(val.t) {
    case VAL_OBJ:   OBJIMPL(optr(val))->unguard(optr(val)); break;
    default:        break;
    }
}

MxcError *new_error(const char *msg) {
    MxcError *ob = (MxcError *)Mxc_malloc(sizeof(MxcError));
    ob->errmsg = msg;

    return ob;
}

MxcIStruct *new_struct(int nfield) {
    MxcIStruct *ob = (MxcIStruct *)Mxc_malloc(sizeof(MxcIStruct));
    ob->field = malloc(sizeof(MxcValue) * nfield);
    return ob;
}

