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

MTable *new_table(MxcString **, MxcValue *, int);

#endif
