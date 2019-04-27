#include "maxc.h"

IntObject *VM::alloc_intobject(int number) {
    IntObject *ob = (IntObject *)malloc(sizeof(IntObject));
    ob->inum32 = number;

    return ob;
}

IntObject *VM::int_add(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 + r->inum32);
}

IntObject *VM::int_sub(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 - r->inum32);
}

IntObject *VM::int_mul(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 * r->inum32);
}

IntObject *VM::int_div(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 / r->inum32);
}

IntObject *VM::int_mod(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 % r->inum32);
}

IntObject *VM::int_logor(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 || r->inum32);
}

IntObject *VM::int_logand(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 && r->inum32);
}

IntObject *VM::int_eq(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 == r->inum32);
}

IntObject *VM::int_noteq(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 != r->inum32);
}

IntObject *VM::int_lt(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 < r->inum32);
}

IntObject *VM::int_lte(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 <= r->inum32);
}

IntObject *VM::int_gt(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 > r->inum32);
}

IntObject *VM::int_gte(IntObject *l, IntObject *r) {
    return alloc_intobject(l->inum32 >= r->inum32);
}

IntObject *VM::int_inc(IntObject *u) {
    return alloc_intobject(++u->inum32);
}

IntObject *VM::int_dec(IntObject *u) {
    return alloc_intobject(--u->inum32);
}

StringObject *VM::alloc_stringobject(const char *s) {
    StringObject *ob = (StringObject *)malloc(sizeof(StringObject));
    ob->str = s;

    return ob;
}

FunctionObject *VM::alloc_functionobject(size_t s) {
    FunctionObject *ob = (FunctionObject *)malloc(sizeof(FunctionObject));
    ob->start = s;

    return ob;
}
