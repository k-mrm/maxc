/* implementation of null object */

#include "object/nullobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcString *null_tostring(MxcObject *ob) {
    INTERN_UNUSE(ob);
    return new_string("null", false);
}

MxcObject *null_copy(MxcObject *s) {
    INCREF(s);
    return s;
}

void null_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

MxcObjImpl null_objimpl = {
    "null",
    null_tostring,
    0,  /* dealloc is never called */
    null_copy,
    null_gc_mark,
    0,
    0,
};

MxcNull _mxc_null = {
    {
        &null_objimpl,
#ifdef USE_MARK_AND_SWEEP 
        0,
#else
        1,  /* refcount */
#endif
    }
};

