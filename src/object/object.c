#include "object/object.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

NullObject MxcNull = {{1}};
BoolObject MxcTrue = {{1}, 1};
BoolObject MxcFalse = {{1}, 0};

IntObject *new_intobject(int64_t number) {
    IntObject *ob = (IntObject *)Mxc_malloc(sizeof(IntObject));
    ob->inum = number;

    return ob;
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
    return new_intobject(l->inum / r->inum);
}

IntObject *int_mod(IntObject *l, IntObject *r) {
    return new_intobject(l->inum % r->inum);
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

FloatObject *new_floatobject(double fnum) {
    FloatObject *ob = (FloatObject *)Mxc_malloc(sizeof(FloatObject));
    ob->fnum = fnum;

    return ob;
}

StringObject *new_stringobject(const char *s) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ob->str = s;

    return ob;
}

StringObject *str_concat(StringObject *a, StringObject *b) {
    int len = strlen(a->str) + strlen(b->str);

    char *res = malloc(sizeof(char) * len + 1);

    strcpy(res, a->str);
    strcat(res, b->str);

    StringObject *new_ob = Mxc_malloc(sizeof(StringObject));

    new_ob->str = res;

    return new_ob;
}

CharObject *new_charobject(char c) {
    CharObject *ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = c;

    return ob;
}

MxcObject *iterable_next(MxcIterable *iter) {
    if(!iter->next) {
        return NULL;
    }

    MxcObject *res = iter->get(iter, iter->index);
    iter->index++;

    return res;
}

MxcObject *list_get(MxcObject *self, size_t idx) {
    return ((ListObject *)self)->elem[idx];
}

ListObject *new_listobject(size_t size) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ((MxcIterable *)ob)->index = 0;
    ((MxcIterable *)ob)->next = NULL;
    ((MxcIterable *)ob)->get = list_get;

    ob->elem = malloc(sizeof(MxcObject *) * size);
    ob->size = size;

    return ob;
}

ErrorObject *new_errorobject(const char *msg) {
    ErrorObject *ob = (ErrorObject *)Mxc_malloc(sizeof(ErrorObject));

    ob->errmsg = msg;

    return ob;
}

FunctionObject *new_functionobject(userfunction *u) {
    FunctionObject *ob = (FunctionObject *)Mxc_malloc(sizeof(FunctionObject));
    ob->func = u;

    return ob;
}

BltinFuncObject *new_bltinfnobject(bltinfn_ty bf) {
    BltinFuncObject *ob =
        (BltinFuncObject *)Mxc_malloc(sizeof(BltinFuncObject));
    ob->func = bf;

    return ob;
}

StructObject *new_structobject(int nfield) {
    StructObject *ob = (StructObject *)Mxc_malloc(sizeof(StructObject));
    ob->field = (MxcObject **)malloc(sizeof(MxcObject *) * nfield);

    return ob;
}

