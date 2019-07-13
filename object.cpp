#include "object.h"
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

BoolObject *bool_logor(BoolObject *l, BoolObject *r) {
    return alloc_boolobject(l->boolean || r->boolean);
}

BoolObject *bool_logand(BoolObject *l, BoolObject *r) {
    return alloc_boolobject(l->boolean && r->boolean);
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
    ++u->inum32;
    return u;
}

IntObject *int_dec(IntObject *u) {
    --u->inum32;
    return u;
}

StringObject *alloc_stringobject(const char *s) {
    auto ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ob->str = s;
    /*
    ob->str = (char *)malloc(sizeof(char) * strlen(s) + 1);
    strncpy(ob->str, s, strlen(s) + 1); */

    return ob;
}

BoolObject *alloc_boolobject(bool b) {
    auto ob = (BoolObject *)Mxc_malloc(sizeof(BoolObject));
    ob->boolean = b;

    return ob;
}

CharObject *alloc_charobject(char c) {
    auto ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = c;

    return ob;
}

ListObject *alloc_listobject(size_t size) {
    auto ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ob->elem = (MxcObject **)malloc(sizeof(MxcObject *) * size);
    ob->size = size;

    return ob;
}

FunctionObject *alloc_functionobject(userfunction u) {
    auto ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    ob->func = u;

    return ob;
}

BltinFuncObject *alloc_bltinfnobject(bltinfn_ty &bf) {
    auto ob = (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;

    return ob;
}

BoolObject *bool_from_int(IntObject *i) {
    if(i->inum32)
        return alloc_boolobject(true);
    else
        return alloc_boolobject(false);
}

MxcObject *Mxc_malloc(size_t s) {
    auto ob = (MxcObject *)malloc(s);
    ob->refcount = 1;

    return ob;
}

} // namespace Object
