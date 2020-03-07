#ifndef MXC_BOOLOBJECT_H
#define MXC_BOOLOBJECT_H

#include <stdint.h>
#include "object/object.h"

typedef struct MxcBool {
    OBJECT_HEAD;
    int64_t boolean;
} MxcBool;

MxcValue bool_logor(MxcValue, MxcValue);
MxcValue bool_logand(MxcValue, MxcValue);
MxcValue bool_not(MxcValue);

#endif
