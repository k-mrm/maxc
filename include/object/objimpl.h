#ifndef MXC_OBJIMPL_H
#define MXC_OBJIMPL_H

#include <stdio.h>

struct MxcObject;
typedef struct MxcObject MxcObject;
struct MxcIterable;
typedef struct MxcIterable MxcIterable;

struct StringObject;
typedef struct StringObject StringObject;

typedef StringObject *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);
typedef MxcObject *(*ob_copy_fn)(MxcObject *);
typedef MxcObject *(*iter_getitem_fn)(MxcIterable *, size_t);
typedef MxcObject *(*iter_setitem_fn)(MxcIterable *, size_t, MxcObject *);

typedef struct MxcObjImpl {
    char *type_name;
    ob_tostring_fn tostring;
    ob_dealloc_fn dealloc;  /* TODO */
    ob_copy_fn copy;
    ob_mark_fn mark;        /* TODO */
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
extern MxcObjImpl bltinfn_objimpl;

#endif
