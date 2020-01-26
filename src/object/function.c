/* implementation of function object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

FunctionObject *new_functionobject(userfunction *u) {
    FunctionObject *ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    ob->func = u;
    OBJIMPL(ob) = &userfn_objimpl;

    return ob;
}

MxcObject *userfn_copy(MxcObject *u) {
    FunctionObject *n = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    memcpy(n, u, sizeof(FunctionObject));
    INCREF(u);

    return n;
}

void userfn_dealloc(MxcObject *ob) {
    free(((FunctionObject *)ob)->func);
    Mxc_free(ob);
}

BltinFuncObject *new_bltinfnobject(bltinfn_ty bf) {
    BltinFuncObject *ob =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;
    OBJIMPL(ob) = &bltinfn_objimpl;

    return ob;
}

MxcObject *bltinfn_copy(MxcObject *b) {
    BltinFuncObject *n =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    memcpy(n, b, sizeof(BltinFuncObject));
    INCREF(b);

    return n;
}

void bltinfn_dealloc(MxcObject *ob) {
    Mxc_free(ob);
}

StringObject *userfn_tostring(MxcObject *ob) {
    char *s = malloc(sizeof(char *) * 64);
    sprintf(s, "<user-def function at %p>", ob);

    return new_stringobject(s, true);
}

StringObject *bltinfn_tostring(MxcObject *ob) {
    (void)ob;
    return new_stringobject("<builtin function>", false);
}

MxcObjImpl userfn_objimpl = {
    "user-def function",
    userfn_tostring,
    userfn_dealloc,
    userfn_copy,
    0,
    0,
    0,
};

MxcObjImpl bltinfn_objimpl = {
    "builtin function",
    bltinfn_tostring,
    bltinfn_dealloc,
    bltinfn_copy,
    0,
    0,
    0,
};

