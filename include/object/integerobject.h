#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    unsigned int size;
    unsigned int *digit;
    char sign;
} MxcInteger;

MxcValue new_integer(char *, int);

#endif
