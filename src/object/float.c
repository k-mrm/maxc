/* implementation of float object */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "object/floatobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcFloat *new_float(double fnum) {
    MxcFloat *ob = (MxcFloat *)Mxc_malloc(sizeof(MxcFloat));
    ob->fnum = fnum;
    OBJIMPL(ob) = &float_objimpl; 

    return ob;
}

MxcObject *float_copy(MxcObject *f) {
    MxcFloat *n = (MxcFloat *)Mxc_malloc(sizeof(MxcFloat));
    memcpy(n, f, sizeof(MxcFloat));

    return (MxcObject *)n;
}

void float_dealloc(MxcObject *ob) {
    Mxc_free(ob);
}

void float_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

MxcBool *float_eq(MxcFloat *l, MxcFloat *r) {
    if(l->fnum == r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *float_neq(MxcFloat *l, MxcFloat *r) {
    if(l->fnum != r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *float_lt(MxcFloat *l, MxcFloat *r) {
    if(l->fnum < r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *float_gt(MxcFloat *l, MxcFloat *r) {
    if(l->fnum > r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcFloat *float_div(MxcFloat *l, MxcFloat *r) {
    if(r->fnum == 0.0) {
        return NULL;
    }

    return new_float(l->fnum / r->fnum);
}

MxcString *float_tostring(MxcObject *ob) {
    double f = ((MxcFloat *)ob)->fnum;
    size_t len = get_digit((int)f) + 10;
    char *str = malloc(sizeof(char) * len);
    sprintf(str, "%.8lf", f);

    return new_string(str, len);
} 

MxcObjImpl float_objimpl = {
    "float",
    float_tostring,
    float_dealloc,
    float_copy,
    float_gc_mark,
    0,
    0,
};
