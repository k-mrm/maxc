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

struct mobj_attr {
  char *attrname;
  size_t offset;
  enum attr_state state;
  Type *type;
};

MxcValue mxc_objattrget(MxcObject *, char *name);
void mxc_objattrset(MxcObject *, char *name, MxcValue src);

struct mobj_attr mxc_objattr(struct mobj_attr *attrs, char *name);

extern struct mobj_attr list_attr[];

#endif
