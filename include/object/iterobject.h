#ifndef MXC_ITEROBJECT_H
#define MXC_ITEROBJECT_H

#include "object/object.h"

struct MxcIterable {
    OBJECT_HEAD;
    MxcValue next;
    size_t length;
    size_t index;
};

MxcValue iterable_next(MxcIterable *);

#endif
