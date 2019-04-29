#include "maxc.h"

namespace Object {
    IntObject *alloc_intobject(int number) {
        auto ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
        ob->inum32 = number;

        return ob;
    }

    IntObject *int_add(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 + r->inum32);
    }

    IntObject *int_sub(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 - r->inum32);
    }

    IntObject *int_mul(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 * r->inum32);
    }

    IntObject *int_div(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 / r->inum32);
    }

    IntObject *int_mod(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 % r->inum32);
    }

    IntObject *int_logor(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 || r->inum32);
    }

    IntObject *int_logand(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 && r->inum32);
    }

    IntObject *int_eq(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 == r->inum32);
    }

    IntObject *int_noteq(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 != r->inum32);
    }

    IntObject *int_lt(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 < r->inum32);
    }

    IntObject *int_lte(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 <= r->inum32);
    }

    IntObject *int_gt(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 > r->inum32);
    }

    IntObject *int_gte(IntObject *l, IntObject *r) {
        return alloc_intobject(l->inum32 >= r->inum32);
    }

    IntObject *int_inc(IntObject *u) {
        return alloc_intobject(++u->inum32);
    }

    IntObject *int_dec(IntObject *u) {
        return alloc_intobject(--u->inum32);
    }

    StringObject *alloc_stringobject(const char *s) {
        auto ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
        ob->str = s;

        return ob;
    }

    ListObject *alloc_listobject(size_t size) {
        auto ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
        ob->elem = (MxcObject **)malloc(sizeof(MxcObject *));
        ob->allocated = size;

        return ob;
    }

    FunctionObject *alloc_functionobject(size_t s) {
        auto ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
        ob->start = s;

        return ob;
    }

    MxcObject *Mxc_malloc(size_t s) {
        auto ob = (MxcObject *)malloc(s);
        if(ob == nullptr) {
            runtime_err("malloc error"); exit(1);
        }
        ob->refcount = 1;

        return ob;
    }

    void incref(MxcObject *ob) {
        ++ob->refcount;
    }

    void decref(MxcObject *ob) {
        if(--ob->refcount == 0) {
            free(ob);
        }
    }
}
