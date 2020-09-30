#ifndef MXC_TABLE_H
#define MXC_TABLE_H

#include <stdint.h>
#include "object/object.h"
#include "object/mstr.h"

struct mentry {
  struct mentry *next;
  MxcValue key;
  MxcValue val;
};

typedef struct MTable MTable;
struct MTable {
  ITERABLE_OBJECT_HEAD;
  int nentry;
  int nslot;
  struct mentry **e;
};

MxcValue new_table_capa(int);
void mtable_add(MTable *, MxcValue, MxcValue);

#endif
