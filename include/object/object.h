#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include <inttypes.h>

#include "object/objimpl.h"

#define OBJECT_HEAD MxcObject base
#define ITERABLE_OBJECT_HEAD MxcIterable base
#define ITERABLE(ob) ((MxcIterable *)(ob))

#define USE_MARK_AND_SWEEP

struct MxcString;
typedef struct MxcString MxcString;
typedef struct MxcObject MxcObject;
typedef struct MxcIterable MxcIterable;

typedef MxcString *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);

#define OBJIMPL(ob) (((MxcObject *)ob)->impl)

struct MxcObject {
    MxcObjImpl *impl;
#ifdef USE_MARK_AND_SWEEP
    unsigned char marked;
#else
    int refcount;
#endif
};

struct MxcValue {
    MxcObjImpl *impl;
    union {
        void *obj;
        int64_t num;
        double fnum;
    };
};

typedef struct MxcError {
    OBJECT_HEAD;
    const char *errmsg;
} MxcError;

typedef struct MxcTuple {
    OBJECT_HEAD;
} MxcTuple; // TODO

typedef struct MxcIStruct {
    OBJECT_HEAD;
    MxcObject **field;
} MxcIStruct;

MxcIStruct *new_struct(int);
MxcError *new_error(const char *);

#endif
