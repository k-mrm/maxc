/* implementation of integer object */
#include <inttypes.h>
#include <stdlib.h> 
#include <string.h>
#include <limits.h>

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

void int_guard(MxcObject *ob) {
    ob->gc_guard = 1;
}

void int_unguard(MxcObject *ob) {
    ob->gc_guard = 0;
}

MxcString *int2str(MxcObject *ob, int base) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    bool neg = false;
    char buf[sizeof(int64_t) * CHAR_BIT + 1];
    char *end = buf + sizeof(buf);
    char *cur = end;
    int64_t num = ((MxcInteger *)ob)->inum;

    if(base < 2 || 36 < base) {
        return NULL;
    }

    if(num < 0) {
        num = -num;
        neg = true;
    }

    do {
        *--cur = digits[num % base];
    } while(num /= base);
    if(neg) {
        *--cur = '-';
    }

    return new_string_copy(cur, end - cur);
}

MxcString *int_tostring(MxcObject *ob) {
    return int2str(ob, 10);
}

MxcObjImpl integer_objimpl = {
    "integer",
    int_tostring,
    int_dealloc,
    int_copy,
    int_gc_mark,
    int_guard,
    int_unguard,
    0,
    0,
};

