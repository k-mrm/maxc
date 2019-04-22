#include "maxc.h"

IntObject *IntObject::add(IntObject *r) {
    this->inum32 += r->inum32;
    return this;
}
