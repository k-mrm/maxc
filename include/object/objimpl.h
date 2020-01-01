#ifndef MXC_OBJIMPL_H
#define MXC_OBJIMPL_H

#include "maxc.h"

struct MxcObject;
typedef struct MxcObject MxcObject;

struct StringObject;
typedef struct StringObject StringObject;

typedef StringObject *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);

typedef struct MxcObjImpl {
    ob_tostring_fn tostring;
    ob_dealloc_fn dealloc;  /* TODO */
    ob_mark_fn mark;        /* TODO */
} MxcObjImpl;

extern MxcObjImpl integer_objimpl;
extern MxcObjImpl bool_true_objimpl;
extern MxcObjImpl bool_false_objimpl;
extern MxcObjImpl list_objimpl;
extern MxcObjImpl userfn_objimpl;
extern MxcObjImpl bltinfn_objimpl;

#endif
