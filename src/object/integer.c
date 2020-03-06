/* implementation of integer object */
#include <inttypes.h>
#include <stdlib.h> 
#include <string.h>
#include <limits.h>

#include "object/intobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcObject *int_copy(MxcObject *i) {
    MxcObject *n = Mxc_malloc(sizeof(MxcInteger));
    memcpy(n, i, sizeof(MxcInteger));

    return n;
}

MxcValue int_add(MxcValue l, MxcValue r) {
    return value_int(l.num + r.num);
}

MxcValue int_sub(MxcValue l, MxcValue r) {
    return value_int(l.num - r.num);
}

MxcValue int_mul(MxcValue l, MxcValue r) {
    return value_int(l.num * r.num);
}

MxcValue int_div(MxcValue l, MxcValue r) {
    if(r.num == 0) {
        return NULL;
    }

    return value_int(l.num / r.num);
}

MxcValue int_mod(MxcValue l, MxcValue r) {
    return value_int(l.num % r.num);
}

MxcValue int_eq(MxcValue l, MxcValue r) {
    if(l.num == r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_noteq(MxcValue l, MxcValue r) {
    if(l.num != r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_lt(MxcValue l, MxcValue r) {
    if(l.num < r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_lte(MxcValue l, MxcValue r) {
    if(l.num <= r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_gt(MxcValue l, MxcValue r) {
    if(l.num > r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_gte(MxcValue l, MxcValue r) {
    if(l.num >= r.num)
        return value_obj(MXC_TRUE);
    else
        return value_obj(MXC_FALSE);
}

MxcValue int_inc(MxcValue u) { return ++u->inum, u; }

MxcValue int_dec(MxcValue u) { return --u->inum, u; }

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

MxcValue int2str(MxcValue val, int base) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    bool neg = false;
    char buf[sizeof(int64_t) * CHAR_BIT + 1];
    char *end = buf + sizeof(buf);
    char *cur = end;
    int64_t num = val.num;

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

    return value_obj(new_string_copy(cur, end - cur));
}

MxcValue int_tostring(MxcValue val) {
    return int2str(val, 10);
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

