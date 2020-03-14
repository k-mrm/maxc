#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    size_t ndigit;
    unsigned int *digit;
    char sign;  /* 1 -> +, 0 -> - */
} MxcInteger;

MxcValue new_integer(char *, int);

#endif
