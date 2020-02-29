#ifndef MXC_OBJIMPL_H
#define MXC_OBJIMPL_H

#include <stdio.h>

struct MxcObject;
typedef struct MxcObject MxcObject;
struct MxcIterable;
typedef struct MxcIterable MxcIterable;

struct MxcString;
typedef struct MxcString MxcString;

typedef MxcString *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);
typedef MxcObject *(*ob_copy_fn)(MxcObject *);
typedef MxcObject *(*iter_getitem_fn)(MxcIterable *, int64_t);
typedef MxcObject *(*iter_setitem_fn)(MxcIterable *, int64_t, MxcObject *);

typedef struct MxcObjImpl {
    char *type_name;
    ob_tostring_fn tostring;
    ob_dealloc_fn dealloc;
    ob_copy_fn copy;
    ob_mark_fn mark;
    ob_mark_fn guard;
    ob_mark_fn unguard;
    iter_getitem_fn get;
    iter_setitem_fn set;
} MxcObjImpl;

extern MxcObjImpl integer_objimpl;
extern MxcObjImpl float_objimpl;
extern MxcObjImpl string_objimpl;
extern MxcObjImpl char_objimpl;
extern MxcObjImpl bool_true_objimpl;
extern MxcObjImpl bool_false_objimpl;
extern MxcObjImpl null_objimpl;
extern MxcObjImpl list_objimpl;
extern MxcObjImpl userfn_objimpl;
extern MxcObjImpl cfn_objimpl;

#endif
