#include "object.h"
#include "error.h"
#include "vm.h"
#include "mem.h"

NullObject Null;
BoolObject MxcTrue;
BoolObject MxcFalse;

namespace Object {

void init() {
    Null.refcount = 1;
    MxcTrue.refcount = 1;
    MxcTrue.boolean = true;
    MxcFalse.refcount = 1;
    MxcFalse.boolean = false;
}

IntObject *alloc_intobject(int64_t number) {
    auto ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
    ob->inum = number;

    return ob;
}

IntObject *int_add(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum + r->inum);
}

IntObject *int_sub(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum - r->inum);
}

IntObject *int_mul(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum * r->inum);
}

IntObject *int_div(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum / r->inum);
}

IntObject *int_mod(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum % r->inum);
}

BoolObject *bool_logor(BoolObject *l, BoolObject *r) {
    if(l->boolean || r->boolean)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *bool_logand(BoolObject *l, BoolObject *r) {
    if(l->boolean && r->boolean)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_eq(IntObject *l, IntObject *r) {
    if(l->inum == r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_noteq(IntObject *l, IntObject *r) {
    if(l->inum != r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_lt(IntObject *l, IntObject *r) {
    if(l->inum < r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_lte(IntObject *l, IntObject *r) {
    if(l->inum <= r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_gt(IntObject *l, IntObject *r) {
    if(l->inum > r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *int_gte(IntObject *l, IntObject *r) {
    if(l->inum >= r->inum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *float_lt(FloatObject *l, FloatObject *r) {
    if(l->fnum < r->fnum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

BoolObject *float_gt(FloatObject *l, FloatObject *r) {
    if(l->fnum > r->fnum)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

IntObject *int_inc(IntObject *u) {
    return ++u->inum, u;
}

IntObject *int_dec(IntObject *u) {
    return --u->inum, u;
}

FloatObject *alloc_floatobject(double fnum) {
    auto ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;

    return ob;
}

StringObject *alloc_stringobject(const char *s) {
    auto ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ob->str = s;
    /*
    ob->str = (char *)malloc(sizeof(char) * strlen(s) + 1);
    strncpy(ob->str, s, strlen(s) + 1); */

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

} // namespace Object
