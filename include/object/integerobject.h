#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include <stdint.h>

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    size_t len;
    uint64_t *digit;
    char sign;  /* 1 -> +, 0 -> - */
} MxcInteger;

MxcValue new_integer(char *, int);

#endif
