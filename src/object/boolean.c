/* implementation of boolean object */

#include "object/boolobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcObject *bool_copy(MxcObject *b) {
    INCREF(b);
    return b;
}

void bool_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

MxcBool *bool_logor(MxcBool *l, MxcBool *r) {
    if(l->boolean || r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *bool_logand(MxcBool *l, MxcBool *r) {
    if(l->boolean && r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *bool_not(MxcBool *u) {
    if(u->boolean)
        MxcBool_RetFalse();
    else
        MxcBool_RetTrue();
}

MxcString *true_tostring(MxcObject *ob) {
    (void)ob;
    return new_string_static("true", 4);
}

MxcString *false_tostring(MxcObject *ob) {
    (void)ob;
    return new_string_static("false", 5);
}

MxcObjImpl bool_true_objimpl = {
    "bool",
    true_tostring,
    0,  /* dealloc is never called */
    bool_copy,
    bool_gc_mark,
    0,
    0,
};

MxcObjImpl bool_false_objimpl = {
    "bool",
    false_tostring,
    0,  /* dealloc is never called */
    bool_copy,
    bool_gc_mark,
    0,
    0,
};

MxcBool _mxc_true = {
    {
        &bool_true_objimpl,
#ifdef USE_MARK_AND_SWEEP
        0,
#else
        1,  /* refcount */
#endif
    },
    true
};

MxcBool _mxc_false = {
    {
        &bool_false_objimpl,
#ifdef USE_MARK_AND_SWEEP
        0,
#else
        1,  /* refcount */
#endif
    },
    false
};

