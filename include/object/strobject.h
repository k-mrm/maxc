#ifndef MXC_STRINGOBJECT_H
#define MXC_STRINGOBJECT_H

#include "object/object.h"
#include "object/iterobject.h"

struct MxcString {
    ITERABLE_OBJECT_HEAD;
    char *str;
    bool isdyn;
};

MxcValue new_string(char *, size_t);
MxcValue new_string_copy(char *, size_t);
MxcValue new_string_static(char *, size_t);
MxcValue str_concat(MxcValue, MxcValue);
void str_append(MxcValue, MxcValue);
void str_cstr_append(MxcValue, char *, size_t);
MxcValue str_index(MxcIterable *, int64_t);
MxcValue str_index_set(MxcIterable *, int64_t, MxcValue);

MxcValue string_tostring(MxcObject *);

#endif
