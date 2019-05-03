#include "maxc.h"

namespace Object {
    IntObject *alloc_intobject(int number) {
        auto ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
        ob->inum32 = number;
        ob->type = CTYPE::INT;

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

    BoolObject *int_logor(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 || r->inum32);
    }

    BoolObject *int_logand(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 && r->inum32);
    }

    BoolObject *int_eq(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 == r->inum32);
    }

    BoolObject *int_noteq(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 != r->inum32);
    }

    BoolObject *int_lt(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 < r->inum32);
    }

    BoolObject *int_lte(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 <= r->inum32);
    }

    BoolObject *int_gt(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 > r->inum32);
    }

    BoolObject *int_gte(IntObject *l, IntObject *r) {
        return alloc_boolobject(l->inum32 >= r->inum32);
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
        /*
        ob->str = (char *)malloc(sizeof(char) * strlen(s) + 1);
        strncpy(ob->str, s, strlen(s) + 1); */
        ob->type = CTYPE::STRING;

        return ob;
    }

    BoolObject *alloc_boolobject(bool b) {
        auto ob = (BoolObject *)Mxc_malloc(sizeof(BoolObject));
        ob->boolean = b;
        ob->type = CTYPE::BOOL;

        return ob;
    }

    ListObject *alloc_listobject(size_t size) {
        auto ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
        ob->elem = (MxcObject **)malloc(sizeof(MxcObject *) * size);
        ob->allocated = size;
        ob->type = CTYPE::LIST;

        return ob;
    }

    FunctionObject *alloc_functionobject(size_t s) {
        auto ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
        ob->start = s;
        ob->type = CTYPE::FUNCTION;

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
