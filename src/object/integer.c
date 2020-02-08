/* implementation of integer object */
#include <inttypes.h>
#include <stdlib.h> 
#include <string.h>

#include "object/intobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

IntObject *new_intobject(int64_t number) {
    IntObject *ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
    ob->inum = number;
    OBJIMPL(ob) = &integer_objimpl;

    return ob;
}

void int_dealloc(MxcObject *i) {
    Mxc_free(i);
}

MxcObject *int_copy(MxcObject *i) {
    MxcObject *n = Mxc_malloc(sizeof(IntObject));
    memcpy(n, i, sizeof(IntObject));

    return n;
}

IntObject *int_add(IntObject *l, IntObject *r) {
    return new_intobject(l->inum + r->inum);
}

IntObject *int_sub(IntObject *l, IntObject *r) {
    return new_intobject(l->inum - r->inum);
}

IntObject *int_mul(IntObject *l, IntObject *r) {
    return new_intobject(l->inum * r->inum);
}

IntObject *int_div(IntObject *l, IntObject *r) {
    if(r->inum == 0) {
        return NULL;
    }

    return new_intobject(l->inum / r->inum);
}

IntObject *int_mod(IntObject *l, IntObject *r) {
    return new_intobject(l->inum % r->inum);
}

BoolObject *int_eq(IntObject *l, IntObject *r) {
    if(l->inum == r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *int_noteq(IntObject *l, IntObject *r) {
    if(l->inum != r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *int_lt(IntObject *l, IntObject *r) {
    if(l->inum < r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *int_lte(IntObject *l, IntObject *r) {
    if(l->inum <= r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *int_gt(IntObject *l, IntObject *r) {
    if(l->inum > r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *int_gte(IntObject *l, IntObject *r) {
    if(l->inum >= r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

IntObject *int_inc(IntObject *u) { return ++u->inum, u; }

IntObject *int_dec(IntObject *u) { return --u->inum, u; }

StringObject *int_tostring(MxcObject *ob) {
    int64_t num = ((IntObject *)ob)->inum;
    char *str = malloc(get_digit(num) * sizeof(char));
    sprintf(str, "%ld", num);

    return new_stringobject(str, true);
}

MxcObjImpl integer_objimpl = {
    "integer",
    int_tostring,
    int_dealloc,
    int_copy,
    0,
    0,
    0,
};

