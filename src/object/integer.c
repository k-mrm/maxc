/* implementation of integer object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

#include <inttypes.h>

IntObject *new_intobject(int64_t number) {
    IntObject *ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
    ob->inum = number;
    ((MxcObject *)ob)->tostring = int_tostring;

    return ob;
}

void int_delete(MxcObject *i) {
    free(i);
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
        runtime_err("division by zero");
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
    sprintf(str, "%lld", num);

    return new_stringobject(str);
}
