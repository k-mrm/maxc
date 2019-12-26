/* implementation of float object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

FloatObject *new_floatobject(double fnum) {
    FloatObject *ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;
    ((MxcObject *)ob)->tostring = float_tostring;

    return ob;
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

StringObject *float_tostring(MxcObject *ob) {
    double f = ((FloatObject *)ob)->fnum;
    char *str = malloc(sizeof(char) * (get_digit((int)f) + 10));
    sprintf(str, "%lf", f);

    return new_stringobject(str);
} 