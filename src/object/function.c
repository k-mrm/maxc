/* implementation of function object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

FunctionObject *new_functionobject(userfunction *u) {
    FunctionObject *ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    ob->func = u;
    ((MxcObject *)ob)->tostring = userfn_tostring;

    return ob;
}

BltinFuncObject *new_bltinfnobject(bltinfn_ty bf) {
    BltinFuncObject *ob =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;
    ((MxcObject *)ob)->tostring = bltinfn_tostring; 

    return ob;
}

StringObject *userfn_tostring(MxcObject *ob) {
    char *s = malloc(sizeof(char *) * 64);
    sprintf(s, "<user-def function at %p>", ob);

    return new_stringobject(s);
}

StringObject *bltinfn_tostring(MxcObject *ob) {
    return new_stringobject("<builtin function>");
}
