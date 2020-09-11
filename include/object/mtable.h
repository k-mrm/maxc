#ifndef MXC_TABLE_H
#define MXC_TABLE_H

#include <stdint.h>
#include "object/object.h"
#include "object/mstr.h"

struct mentry {
  uint32_t key;
  MxcValue val;
};

typedef struct MTable MTable;
struct MTable {
  OBJECT_HEAD;
  int len;
  int capa;
  struct mentry **e;
};

MxcValue new_table(MxcString **, MxcValue *, int);
MxcValue new_table_capa(int);
void mtable_add(MTable *, MxcValue, MxcValue);

#endif
