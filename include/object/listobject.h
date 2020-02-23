#ifndef MXC_LISTOBJECT_H
#define MXC_LISTOBJECT_H

#include "object/object.h"
#include "object/iterobject.h"

struct MxcInteger;
typedef struct MxcInteger MxcInteger;

typedef struct MxcList {
    ITERABLE_OBJECT_HEAD;
    MxcObject **elem;
} MxcList;

MxcList *new_list(size_t);
MxcList *new_list_with_size(MxcInteger *, MxcObject *);

MxcObject *list_get(MxcIterable *, size_t);
MxcObject *list_set(MxcIterable *, size_t, MxcObject *);

MxcString *list_tostring(MxcObject *);

#endif
