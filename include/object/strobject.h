#ifndef MXC_STRINGOBJECT_H
#define MXC_STRINGOBJECT_H

#include "object/object.h"
#include "object/iterobject.h"

struct MxcString {
    ITERABLE_OBJECT_HEAD;
    char *str;
    bool isdyn;
};

MxcString *new_string(char *, size_t);
MxcString *new_string_copy(char *, size_t);
MxcString *new_string_static(char *, size_t);
MxcString *str_concat(MxcString *, MxcString *);
MxcObject *str_index(MxcIterable *, size_t);
MxcObject *str_index_set(MxcIterable *, size_t, MxcObject *);

MxcString *string_tostring(MxcObject *);

#endif
