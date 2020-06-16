#ifndef MXC_SYSTEM_H
#define MXC_SYSTEM_H

#include <stdio.h>

struct MxcObject;
typedef struct MxcObject MxcObject;
struct MxcValue;
typedef struct MxcValue MxcValue;
struct MxcIterable;
typedef struct MxcIterable MxcIterable;

struct MxcString;
typedef struct MxcString MxcString;

typedef MxcValue (*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);
typedef MxcValue (*ob_copy_fn)(MxcObject *);
typedef MxcValue (*iter_getitem_fn)(MxcIterable *, int64_t);
typedef MxcValue (*iter_setitem_fn)(MxcIterable *, int64_t, MxcValue);

struct mobj_system {
  char *type_name;
  ob_tostring_fn tostring;
  ob_dealloc_fn dealloc;
  ob_copy_fn copy;
  ob_mark_fn mark;
  ob_mark_fn guard;
  ob_mark_fn unguard;
  iter_getitem_fn get;
  iter_setitem_fn set;
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

#endif
