/* implementation of boolean object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
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

BoolObject MxcTrue = {
    {
        1,
        true_tostring
    },
    1
};

BoolObject MxcFalse = {
    {
        1,
        false_tostring
    },
    0
};
