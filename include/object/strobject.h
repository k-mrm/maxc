#ifndef MXC_STRINGOBJECT_H
#define MXC_STRINGOBJECT_H

#include "object/object.h"
#include "object/iterobject.h"

struct StringObject {
    ITERABLE_OBJECT_HEAD;
    char *str;
    bool isdyn;
};

StringObject *new_stringobject(char *, bool);

StringObject *str_concat(StringObject *, StringObject *);
MxcObject *str_index(MxcIterable *, size_t);
MxcObject *str_index_set(MxcIterable *, size_t, MxcObject *);

#endif
