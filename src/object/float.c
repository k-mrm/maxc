/* implementation of float object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

FloatObject *new_floatobject(double fnum) {
    FloatObject *ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;
    OBJIMPL(ob) = &float_objimpl; 

    return ob;
}

void float_dealloc(MxcObject *ob) {
    free(ob);
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
        runtime_err("division by zero");
    }

    return new_floatobject(l->fnum / r->fnum);
}

StringObject *float_tostring(MxcObject *ob) {
    double f = ((FloatObject *)ob)->fnum;
    char *str = malloc(sizeof(char) * (get_digit((int)f) + 10));
    sprintf(str, "%lf", f);

    return new_stringobject(str);
} 

MxcObjImpl float_objimpl = {
    "float",
    float_tostring,
    float_dealloc,
    0,
};
