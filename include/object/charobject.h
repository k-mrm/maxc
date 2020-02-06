#ifndef MXC_CHAROBJECT_H
#define MXC_CHAROBJECT_H

#include "object/object.h"

typedef struct CharObject {
    OBJECT_HEAD;
    char ch;
} CharObject;

CharObject *new_charobject(char);
CharObject *new_charobject_ref(char *c);

#endif
