#ifndef MXC_LISTOBJECT_H
#define MXC_LISTOBJECT_H

#include "object/object.h"
#include "object/iterobject.h"

struct IntObject;
typedef struct IntObject IntObject;

typedef struct ListObject {
    ITERABLE_OBJECT_HEAD;
    MxcObject **elem;
} ListObject;

ListObject *new_listobject(size_t);
ListObject *new_listobject_size(IntObject *, MxcObject *);

MxcObject *list_get(MxcIterable *, size_t);
MxcObject *list_set(MxcIterable *, size_t, MxcObject *);

#endif
