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
