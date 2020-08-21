#ifndef MXC_STRINGOBJECT_H
#define MXC_STRINGOBJECT_H

#include <stdbool.h>
#include "object/object.h"
#include "object/iterobject.h"
#include "context.h"

struct MxcString {
  ITERABLE_OBJECT_HEAD;
  char *str;
  bool isdyn;
};

MxcValue new_string(char *, size_t);
MxcValue new_string_copy(char *, size_t);
MxcValue new_string_static(char *, size_t);
MxcValue str_concat(MxcString *, MxcString *);
void str_append(MxcString *, MxcString *);
void str_cstr_append(MxcString *, char *, size_t);
MxcValue str_index(MxcIterable *, int64_t);
MxcValue str_index_set(MxcIterable *, int64_t, MxcValue);
MxcValue mstr_eq(MContext *, MxcValue *, size_t);

MxcValue string_tostring(MxcObject *);

#endif
