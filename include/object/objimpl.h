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
    ob_dealloc_fn dealloc;
    ob_mark_fn mark;
} MxcObjImpl;

#endif