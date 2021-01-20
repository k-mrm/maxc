#include <string.h>
#include "object/object.h"
#include "object/attr.h"
#include "object/system.h"

MxcValue mxc_objattrget(MxcObject *ob, char *name) {
  struct mobj_attr *attr = SYSTEM(ob)->attr;
  for(struct mobj_attr a = *attr; a.attrname; a = *++attr) {
    if(!strcmp(a.attrname, name)) {
      return *(MxcValue *)(ob + a.offset);
    }
  }
  /* raise error */
}
void mxc_objattrset(MxcObject *ob, char *name, MxcValue src) {
  struct mobj_attr *attr = SYSTEM(ob)->attr;
  for(struct mobj_attr a = *attr; a.attrname; a = *++attr) {
    if(!strcmp(a.attrname, name)) {
      *(MxcValue *)(ob + a.offset) = src;
    }
  }
}

struct mobj_attr mxc_objattr(struct mobj_attr *attr, char *name) {
  for(struct mobj_attr a = *attr; a.attrname; a = *++attr) {
    if(!strcmp(a.attrname, name)) {
      return a;
    }
  }
  return (struct mobj_attr){NULL};
}
