/* implementation of float object */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "object/floatobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue float_copy(MxcValue v) {
    return v;
}

MxcValue float_eq(MxcValue l, MxcValue r) {
    if(l.fnum == r.fnum)
        return mval_true;
    else
        return mval_false;
}

MxcValue float_neq(MxcValue l, MxcValue r) {
    if(l.fnum != r.fnum)
        return mval_true;
    else
        return mval_false;
}

MxcValue float_lt(MxcValue l, MxcValue r) {
    if(l.fnum < r.fnum)
        return mval_true;
    else
        return mval_false;
}

MxcValue float_lte(MxcValue l, MxcValue r) {
    if(l.fnum <= r.fnum)
        return mval_true;
    else
        return mval_false;
}

MxcValue float_gt(MxcValue l, MxcValue r) {
    if(l.fnum > r.fnum)
        return mval_true;
    else
        return mval_false;
}

MxcValue float_div(MxcValue l, MxcValue r) {
    if(r.fnum == 0.0) {
        return mval_invalid;
    }

    return mval_float(l.fnum / r.fnum);
}

MxcValue float_tostring(MxcValue val) {
    double f = val.fnum;
    size_t len = get_digit((int)f) + 10;
    char *str = malloc(sizeof(char) * len);
    sprintf(str, "%.8lf", f);

    return new_string(str, len);
} 

