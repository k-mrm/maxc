#ifndef MXC_SYSTEM_H
#define MXC_SYSTEM_H

#include <stdio.h>
#include <stdbool.h>
#include "object/object.h"
#include "object/attr.h"

struct mobj_system {
  char *type_name;
  struct mobj_attr *attr;

  MxcValue (*tostring)(MxcObject *);
  void (*dealloc)(MxcObject *);
  MxcValue (*copy)(MxcObject *);
  void (*mark)(MxcObject *);
  void (*guard)(MxcObject *);
  void (*unguard)(MxcObject *);
  MxcValue (*get)(MxcIterable *, MxcValue);
  MxcValue (*set)(MxcIterable *, MxcValue, MxcValue);
  MxcValue (*getiter)(MxcObject *);
  MxcValue (*iter_next)(MxcObject *);
  MxcValue (*iter_stopped)(MxcObject *);
  uint32_t (*hash)(MxcObject *);
  bool (*eq)(MxcObject *, MxcObject *);
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
extern struct mobj_system dir_sys;
extern struct mobj_system stat_sys;
extern struct mobj_system time_sys;

#endif
