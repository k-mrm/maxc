#include <stdlib.h>

#include "object/object.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue val2str(MxcValue val) {
    switch(val.t) {
    case VAL_INT:
        return int_tostring(val);
    case VAL_FLO:
        return float_tostring(val);
    case VAL_OBJ:
        return val.obj->tostring(val.obj);
    default:
        error("unreachable");
    }

    return value_invalid();
}

MxcError *new_error(const char *msg) {
    MxcError *ob = (MxcError *)Mxc_malloc(sizeof(MxcError));
    ob->errmsg = msg;

    return ob;
}

MxcIStruct *new_struct(int nfield) {
    MxcIStruct *ob = (MxcIStruct *)Mxc_malloc(sizeof(MxcIStruct));
    ob->field = (MxcObject **)malloc(sizeof(MxcObject *) * nfield);
    return ob;
}

