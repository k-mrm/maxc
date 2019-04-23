#include "maxc.h"

IntObject *IntObject::add(IntObject *r) {
    auto a = this->inum32 + r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::sub(IntObject *r) {
    auto a = this->inum32 - r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::mul(IntObject *r) {
    auto a = this->inum32 * r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::div(IntObject *r) {
    auto a = this->inum32 / r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::mod(IntObject *r) {
    auto a = this->inum32 % r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::logor(IntObject *r) {
    auto a = this->inum32 || r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::logand(IntObject *r) {
    auto a = this->inum32 && r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::eq(IntObject *r) {
    auto a = this->inum32 == r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::noteq(IntObject *r) {
    auto a = this->inum32 != r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::lt(IntObject *r) {
    auto a = this->inum32 < r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::lte(IntObject *r) {
    auto a = this->inum32 <= r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::gt(IntObject *r) {
    auto a = this->inum32 > r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::gte(IntObject *r) {
    auto a = this->inum32 >= r->inum32;
    return new IntObject(a);
}

IntObject *IntObject::inc() {
    ++(this->inum32);
    return this;
}

IntObject *IntObject::dec() {
    --(this->inum32);
    return this;
}
