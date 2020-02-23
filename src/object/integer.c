/* implementation of integer object */
#include <inttypes.h>
#include <stdlib.h> 
#include <string.h>

#include "object/intobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcInteger *new_int(int64_t number) {
    MxcInteger *ob = (MxcInteger *)Mxc_malloc(sizeof(MxcInteger));
    ob->inum = number;
    OBJIMPL(ob) = &integer_objimpl;

    return ob;
}

void int_dealloc(MxcObject *i) {
    Mxc_free(i);
}

MxcObject *int_copy(MxcObject *i) {
    MxcObject *n = Mxc_malloc(sizeof(MxcInteger));
    memcpy(n, i, sizeof(MxcInteger));

    return n;
}

MxcInteger *int_add(MxcInteger *l, MxcInteger *r) {
    return new_int(l->inum + r->inum);
}

MxcInteger *int_sub(MxcInteger *l, MxcInteger *r) {
    return new_int(l->inum - r->inum);
}

MxcInteger *int_mul(MxcInteger *l, MxcInteger *r) {
    return new_int(l->inum * r->inum);
}

MxcInteger *int_div(MxcInteger *l, MxcInteger *r) {
    if(r->inum == 0) {
        return NULL;
    }

    return new_int(l->inum / r->inum);
}

MxcInteger *int_mod(MxcInteger *l, MxcInteger *r) {
    return new_int(l->inum % r->inum);
}

MxcBool *int_eq(MxcInteger *l, MxcInteger *r) {
    if(l->inum == r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *int_noteq(MxcInteger *l, MxcInteger *r) {
    if(l->inum != r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *int_lt(MxcInteger *l, MxcInteger *r) {
    if(l->inum < r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *int_lte(MxcInteger *l, MxcInteger *r) {
    if(l->inum <= r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *int_gt(MxcInteger *l, MxcInteger *r) {
    if(l->inum > r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcBool *int_gte(MxcInteger *l, MxcInteger *r) {
    if(l->inum >= r->inum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

MxcInteger *int_inc(MxcInteger *u) { return ++u->inum, u; }

MxcInteger *int_dec(MxcInteger *u) { return --u->inum, u; }

void int_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

MxcString *int_tostring(MxcObject *ob) {
    int64_t num = ((MxcInteger *)ob)->inum;
    size_t len = get_digit(num) + 1;
    char *str = malloc(len * sizeof(char));
    sprintf(str, "%ld", num);

    return new_string(str, len);
}

MxcObjImpl integer_objimpl = {
    "integer",
    int_tostring,
    int_dealloc,
    int_copy,
    int_gc_mark,
    0,
    0,
};

