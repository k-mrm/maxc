#ifndef MXC_LISTOBJECT_H
#define MXC_LISTOBJECT_H

#include "object/object.h"
#include "object/miter.h"

#define LISTLEN(l) (ITERABLE((l))->length)
#define LISTCAPA(l) ((l)->capa)

typedef struct MList {
  ITERABLE_OBJECT_HEAD;
  int capa;
  MxcValue *elem;
} MList;

MxcValue new_list(size_t);
MxcValue new_list2(MxcValue *es, size_t nes);
MxcValue new_list_size(MxcValue, MxcValue);
MxcValue mlistset(MList *l, MxcValue idx, MxcValue a);
MxcValue mlistget(MList *l, MxcValue idx);
MxcValue listadd(MList *, MxcValue);

MxcValue list_tostring(MxcObject *);

#endif
