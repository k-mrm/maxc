/* implementation of null object */

#include "object/nullobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcString *null_tostring(MxcObject *ob) {
    INTERN_UNUSE(ob);
    return new_string_static("null", 4);
}

MxcObject *null_copy(MxcObject *s) {
    INCREF(s);
    return s;
}

void null_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

void null_guard(MxcObject *ob) {
    INTERN_UNUSE(ob);
    /* do nothing */
}

void null_unguard(MxcObject *ob) {
    INTERN_UNUSE(ob);
    /* do nothing */
}

MxcObjImpl null_objimpl = {
    "null",
    null_tostring,
    0,  /* dealloc is never called */
    null_copy,
    null_gc_mark,
    null_guard,
    null_unguard,
    0,
    0,
};

MxcNull _mxc_null = {
    {
        &null_objimpl,
#ifdef USE_MARK_AND_SWEEP 
        0,
        1,
#else
        1,  /* refcount */
#endif
    }
};

