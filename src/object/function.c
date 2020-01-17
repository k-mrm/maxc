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

void userfn_dealloc(MxcObject *ob) {
    free(ob);
}

BltinFuncObject *new_bltinfnobject(bltinfn_ty bf) {
    BltinFuncObject *ob =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;
    OBJIMPL(ob) = &bltinfn_objimpl;

    return ob;
}

void bltinfn_dealloc(MxcObject *ob) {
    free(ob);
}

StringObject *userfn_tostring(MxcObject *ob) {
    char *s = malloc(sizeof(char *) * 64);
    sprintf(s, "<user-def function at %p>", ob);

    return new_stringobject(s);
}

StringObject *bltinfn_tostring(MxcObject *ob) {
    (void)ob;
    return new_stringobject("<builtin function>");
}

MxcObjImpl userfn_objimpl = {
    "user-def function",
    userfn_tostring,
    userfn_dealloc,
    0
};

MxcObjImpl bltinfn_objimpl = {
    "builtin function",
    bltinfn_tostring,
    bltinfn_dealloc,
    0
};

