/* implementation of null object */

#include "object/nullobject.h"
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
    0,  /* dealloc is never called */
    null_copy,
    0,
    0,
    0,
};

NullObject MxcNull = {
    {
        &null_objimpl,
#ifdef USE_MARK_AND_SWEEP 
        0,
#else
        1,  /* refcount */
#endif
    }
};

