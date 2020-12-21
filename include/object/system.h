#ifndef MXC_SYSTEM_H
#define MXC_SYSTEM_H

#include <stdio.h>
#include "object/object.h"

typedef MxcValue (*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);
typedef MxcValue (*ob_copy_fn)(MxcObject *);
typedef MxcValue (*getiter_fn)(MxcObject *);
typedef MxcValue (*iternext_fn)(MxcObject *);
typedef MxcValue (*iterstop_fn)(MxcObject *);
typedef MxcValue (*getitem_fn)(MxcIterable *, MxcValue);
typedef MxcValue (*setitem_fn)(MxcIterable *, MxcValue, MxcValue);
typedef uint32_t (*hash_fn)(MxcObject *);

struct mobj_system {
  char *type_name;
  ob_tostring_fn tostring;
  ob_dealloc_fn dealloc;
  ob_copy_fn copy;
  ob_mark_fn mark;
  ob_mark_fn guard;
  ob_mark_fn unguard;
  getitem_fn get;
  setitem_fn set;
  getiter_fn getiter;
  iternext_fn iter_next;
  iterstop_fn iter_stopped;
  hash_fn hash;
};

extern struct mobj_system integer_sys;
extern struct mobj_system float_sys;
extern struct mobj_system string_sys;
extern struct mobj_system char_sys;
extern struct mobj_system bool_true_sys;
extern struct mobj_system bool_false_sys;
extern struct mobj_system null_sys;
extern struct mobj_system list_sys;
extern struct mobj_system userfn_sys;
extern struct mobj_system cfn_sys;
extern struct mobj_system fiber_sys;
extern struct mobj_system table_sys;

#endif
