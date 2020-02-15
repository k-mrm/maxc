/* implementation of function object */
#include <string.h>
#include <stdlib.h>

#include "object/funcobject.h"
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

    return (MxcObject *)n;
}

void userfn_dealloc(MxcObject *ob) {
    free(((FunctionObject *)ob)->func);
    Mxc_free(ob);
}

MxcObject *userfn_call(MxcObject *self,
                       Frame *f,
                       MxcObject **args,
                       size_t nargs) {
    FunctionObject *callee = (FunctionObject *)self;
    Frame *new_frame = New_Frame(callee->func, f);

    int res = vm_exec(new_frame);

    Delete_Frame(new_frame);

    if(res) return NULL;

    Mxc_RetNull();
}


CFuncObject *new_cfnobject(CFunction cf) {
    CFuncObject *ob =
        (CFuncObject *)Mxc_malloc(sizeof(CFuncObject));
    ob->func = cf;
    OBJIMPL(ob) = &cfn_objimpl;

    return ob;
}

MxcObject *cfn_copy(MxcObject *b) {
    CFuncObject *n =
        (CFuncObject *)Mxc_malloc(sizeof(CFuncObject));
    memcpy(n, b, sizeof(CFuncObject));
    INCREF(b);

    return (MxcObject *)n;
}

MxcObject *cfn_call(MxcObject *self,
                    Frame *f,
                    MxcObject **args,
                    size_t nargs) {
    CFuncObject *callee = (CFuncObject *)self;
    return callee->func(f, args, nargs);
}

void cfn_dealloc(MxcObject *ob) {
    Mxc_free(ob);
}

StringObject *userfn_tostring(MxcObject *ob) {
    char *s = malloc(sizeof(char *) * 64);
    sprintf(s, "<user-def function at %p>", ob);

    return new_stringobject(s, true);
}

StringObject *cfn_tostring(MxcObject *ob) {
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

MxcObjImpl cfn_objimpl = {
    "builtin function",
    cfn_tostring,
    cfn_dealloc,
    cfn_copy,
    0,
    0,
    0,
};

