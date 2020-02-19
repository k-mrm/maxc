/* implementation of float object */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "object/floatobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

FloatObject *new_floatobject(double fnum) {
    FloatObject *ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;
    OBJIMPL(ob) = &float_objimpl; 

    return ob;
}

MxcObject *float_copy(MxcObject *f) {
    FloatObject *n = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    memcpy(n, f, sizeof(FloatObject));

    return (MxcObject *)n;
}

void float_dealloc(MxcObject *ob) {
    Mxc_free(ob);
}

void float_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

BoolObject *float_eq(FloatObject *l, FloatObject *r) {
    if(l->fnum == r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *float_neq(FloatObject *l, FloatObject *r) {
    if(l->fnum != r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *float_lt(FloatObject *l, FloatObject *r) {
    if(l->fnum < r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *float_gt(FloatObject *l, FloatObject *r) {
    if(l->fnum > r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

FloatObject *float_div(FloatObject *l, FloatObject *r) {
    if(r->fnum == 0.0) {
        return NULL;
    }

    return new_floatobject(l->fnum / r->fnum);
}

StringObject *float_tostring(MxcObject *ob) {
    double f = ((FloatObject *)ob)->fnum;
    char *str = malloc(sizeof(char) * (get_digit((int)f) + 10));
    sprintf(str, "%.8lf", f);

    return new_stringobject(str, true);
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
