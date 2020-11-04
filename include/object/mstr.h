#ifndef MXC_STRINGOBJECT_H
#define MXC_STRINGOBJECT_H

#include <stdbool.h>
#include "object/object.h"
#include "object/miter.h"

#define STRLEN(s) (ITERABLE(s)->length)

struct MString {
  ITERABLE_OBJECT_HEAD;
  char *str;
  bool isdyn;
};

MxcValue new_string(char *, size_t);
MxcValue new_string_copy(char *, size_t);
MxcValue new_string_static(char *, size_t);
MxcValue str_concat(MString *, MString *);
void str_append(MString *, MString *);
void str_cstr_append(MString *, char *, size_t);
MxcValue mstr_eq(MxcValue *, size_t);

MxcValue string_tostring(MxcObject *);

#endif
