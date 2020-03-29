#ifndef MXC_INTEGEROBJECT_H
#define MXC_INTEGEROBJECT_H

#include <stdint.h>

#include "object/object.h"

#define DIGIT_POW (32)
#define DIGIT_BASE ((digit2_t)1 << DIGIT_POW)
#define DIGIT_MAX (DIGIT_BASE - 1)

typedef unsigned int digit_t;
typedef unsigned long long digit2_t;
typedef signed long long sdigit2_t;

enum {
    SIGN_MINUS = 0,
    SIGN_PLUS  = 1,
};

typedef struct MxcInteger {
    OBJECT_HEAD;
    size_t len;
    digit_t *digit;
    char sign;
} MxcInteger;

MxcValue new_integer(char *, int);
MxcValue integer_add(MxcValue a, MxcValue b);
MxcValue integer_sub(MxcValue a, MxcValue b);

#endif
