#ifndef MXC_OBJECT_ATTR_H
#define MXC_OBJECT_ATTR_H

#include <stdio.h>
#include <stddef.h>
#include "object/object.h"
#include "type.h"

enum attr_state {
  ATTR_READABLE = 0x1,
  ATTR_WRITABLE = 0x2,
};

enum attr_type {
  ATTY_CINT,
  ATTY_CFLOAT,
  ATTY_CBOOL,
  ATTY_MVALUE,
};

struct mobj_attr {
  char *attrname;
  size_t offset;
  enum attr_state state;
  enum attr_type attype;
  Type *type;
  size_t ty_memb_off;
};

MxcValue mxc_objattrget(MxcObject *, char *name);
void mxc_objattrset(MxcObject *, char *name, MxcValue src);

struct mobj_attr mxc_objattr(struct mobj_attr *attrs, char *name);

extern struct mobj_attr list_attr[];
extern struct mobj_attr table_attr[];

#endif
