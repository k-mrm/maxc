#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include <stdint.h>

#include "object/object.h"

#define INTEGER_BASE (32)

enum {
    SIGN_MINUS = 0,
    SIGN_PLUS  = 1,
};

typedef struct MxcInteger {
    OBJECT_HEAD;
    size_t len;
    unsigned int *digit;
    char sign;
} MxcInteger;

MxcValue new_integer(char *, int);

#endif
