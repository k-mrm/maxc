#include "object.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

NullObject MxcNull = {{1}};
BoolObject MxcTrue = {{1}, true};
BoolObject MxcFalse = {{1}, false};

IntObject *alloc_intobject(int64_t number) {
    IntObject *ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
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
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *bool_logand(BoolObject *l, BoolObject *r) {
    if(l->boolean && r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
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

BoolObject *float_lt(FloatObject *l, FloatObject *r) {
    if(l->fnum < r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *float_gt(FloatObject *l, FloatObject *r) {
    if(l->fnum > r->fnum)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

IntObject *int_inc(IntObject *u) { return ++u->inum, u; }

IntObject *int_dec(IntObject *u) { return --u->inum, u; }

FloatObject *alloc_floatobject(double fnum) {
    FloatObject *ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;

    return ob;
}

StringObject *alloc_stringobject(const char *s) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ob->str = s;

    return ob;
}

CharObject *alloc_charobject(char c) {
    CharObject *ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = c;

    return ob;
}

ListObject *alloc_listobject(size_t size) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ob->elem = (MxcObject **)malloc(sizeof(MxcObject *) * size);
    ob->size = size;

    return ob;
}

FunctionObject *alloc_functionobject(userfunction *u) {
    FunctionObject *ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    ob->func = u;

    return ob;
}

BltinFuncObject *alloc_bltinfnobject(bltinfn_ty bf) {
    BltinFuncObject *ob =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;

    return ob;
}

StructObject *alloc_structobject(int nfield) {
    StructObject *ob = (StructObject *)Mxc_malloc(sizeof(StructObject));
    ob->field = (MxcObject **)malloc(sizeof(MxcObject *) * nfield);

    return ob;
}
