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
typedef struct MxcValue MxcValue;

#define OBJIMPL(ob) (((MxcObject *)ob)->impl)

struct MxcObject {
    MxcObjImpl *impl;
    unsigned char marked;
    unsigned char gc_guard;
};

enum VALUET {
    VAL_INT,
    VAL_FLO,
    VAL_BOOL,
    VAL_NULL,
    VAL_OBJ,
    VAL_INVALID = -1,
};

struct MxcValue {
    enum VALUET t;
    union {
        MxcObject *obj;
        int64_t num;
        double fnum;
    };
};

#define mval_int(v)    (MxcValue){ .t = VAL_INT, .num = (v) }
#define mval_float(v)  (MxcValue){ .t = VAL_FLO, .fnum = (v) }
#define mval_obj(v)    (MxcValue){ .t = VAL_OBJ, .obj = (MxcObject *)(v) }
#define mval_invalid   (MxcValue){ .t = VAL_INVALID, {0}}

#define optr(v) (v.obj)
#define ostr(v) ((MxcString *)v.obj)

MxcValue val2str(MxcValue);
void gc_mark(MxcValue);

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
