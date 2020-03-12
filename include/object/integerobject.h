#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    size_t len;
    unsigned int *digit;
    unsigned char sign;
} MxcInteger;

#endif
