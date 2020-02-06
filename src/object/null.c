/* implementation of null object */

#include "object/nullobject.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

StringObject *null_tostring(MxcObject *ob) {
    INTERN_UNUSE(ob);
    return new_stringobject("null", false);
}

MxcObject *null_copy(MxcObject *s) {
    INCREF(s);
    return s;
}

MxcObjImpl null_objimpl = {
    "null",
    null_tostring,
    0,
    null_copy,
    0,
    0,
    0,
};

NullObject MxcNull = {
    {
        &null_objimpl,
        1,  /* refcount */
        0,
    }
};

