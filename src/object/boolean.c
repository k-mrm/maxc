/* implementation of boolean object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

BoolObject *bool_logor(BoolObject *l, BoolObject *r) {
    if(l->boolean || r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *bool_logand(BoolObject *l, BoolObject *r) {
    if(l->boolean && r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

StringObject *true_tostring(MxcObject *ob) {
    return new_stringobject("true");
}

StringObject *false_tostring(MxcObject *ob) {
    return new_stringobject("false");
}

MxcObjImpl bool_true_objimpl = {
    "bool",
    true_tostring,
    0,
    0
};

MxcObjImpl bool_false_objimpl = {
    "bool",
    false_tostring,
    0,
    0
};

BoolObject MxcTrue = {
    {
        &bool_true_objimpl,
        1,  /* refcount */
        0
    },
    1
};

BoolObject MxcFalse = {
    {
        &bool_false_objimpl,
        1,  /* refcount */
        0
    },
    0
};

