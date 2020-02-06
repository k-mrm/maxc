#ifndef MXC_ITEROBJECT_H
#define MXC_ITEROBJECT_H

#include "object/object.h"

struct MxcIterable {
    OBJECT_HEAD;
    MxcObject *next;
    size_t length;
    size_t index;
};

MxcObject *iterable_next(MxcIterable *);

#endif
