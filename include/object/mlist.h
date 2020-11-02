#ifndef MXC_LISTOBJECT_H
#define MXC_LISTOBJECT_H

#include "object/object.h"
#include "object/miter.h"

struct MxcInteger;
typedef struct MxcInteger MxcInteger;

#define LISTLEN(l) (ITERABLE((l))->length)
#define LISTCAPA(l) ((l)->capa)

typedef struct MList {
  ITERABLE_OBJECT_HEAD;
  int capa;
  MxcValue *elem;
} MList;

MxcValue new_list(size_t);
MxcValue new_list_size(MxcValue, MxcValue);
MxcValue listadd(MList *, MxcValue);

MxcValue list_tostring(MxcObject *);

#endif
