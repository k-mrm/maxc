#ifndef MXC_LISTOBJECT_H
#define MXC_LISTOBJECT_H

#include "object/object.h"
#include "object/miter.h"

struct MxcInteger;
typedef struct MxcInteger MxcInteger;

typedef struct MxcList {
  ITERABLE_OBJECT_HEAD;
  MxcValue *elem;
} MxcList;

MxcValue new_list(size_t);
MxcValue new_list_size(MxcValue, MxcValue);

MxcValue list_tostring(MxcObject *);

#endif
