#ifndef MXC_TABLE_H
#define MXC_TABLE_H

#include <stdint.h>
#include "object/object.h"
#include "object/mstr.h"

struct mentry {
  uint32_t key;
  MxcValue val;
};

struct MTable {
  OBJECT_HEAD;
  int len;
  int capa;
  MxcValue default;
  struct mentry **e;
};

MTable *new_table(MxcString **, MxcValue *, int);

#endif
