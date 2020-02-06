#ifndef MXC_BOOLOBJECT_H
#define MXC_BOOLOBJECT_H

#include <stdint.h>
#include "object/object.h"

typedef struct BoolObject {
    OBJECT_HEAD;
    int64_t boolean;
} BoolObject;

BoolObject *bool_logor(BoolObject *, BoolObject *);
BoolObject *bool_logand(BoolObject *, BoolObject *);
BoolObject *bool_not(BoolObject *);

extern BoolObject MxcTrue;
extern BoolObject MxcFalse;

#define MXC_TRUE  ((MxcObject *)&MxcTrue)
#define MXC_FALSE ((MxcObject *)&MxcFalse)

#define Mxc_RetTrue() return INCREF(&MxcTrue), MXC_TRUE
#define Mxc_RetFalse() return INCREF(&MxcFalse), MXC_FALSE

#define MxcBool_RetTrue() return INCREF(&MxcTrue), &MxcTrue
#define MxcBool_RetFalse() return INCREF(&MxcFalse), &MxcFalse

#endif
