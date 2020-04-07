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
    VAL_TRUE,
    VAL_FALSE,
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
#define mval_true      (MxcValue){ .t = VAL_TRUE, .num = 1 }
#define mval_false     (MxcValue){ .t = VAL_FALSE, .num = 0 }
#define mval_null      (MxcValue){ .t = VAL_NULL, .num = 0 }
#define mval_obj(v)    (MxcValue){ .t = VAL_OBJ, .obj = (MxcObject *)(v) }
#define mval_obj_dummy (MxcValue){ .t = VAL_OBJ, .obj = 0 }
#define mval_invalid   (MxcValue){ .t = VAL_INVALID, {0}}

#define Invalid_val(v)  ((v).t == VAL_INVALID)
#define isobj(v)        ((v).t == VAL_OBJ)

#define optr(v)     (v.obj)
#define oint(v)     ((MxcInteger *)v.obj)
#define ostr(v)     ((MxcString *)v.obj)
#define ocallee(v)  ((MxcCallable *)v.obj)
#define olist(v)    ((MxcList *)v.obj)
#define ostrct(v)   ((MxcIStruct *)v.obj)

MxcValue mval2str(MxcValue);
MxcValue mval_copy(MxcValue);
void mgc_mark(MxcValue);
void mgc_guard(MxcValue);
void mgc_unguard(MxcValue);

typedef struct MxcError {
    OBJECT_HEAD;
    const char *errmsg;
} MxcError;

typedef struct MxcTuple {
    OBJECT_HEAD;
} MxcTuple; // TODO

typedef struct MxcIStruct {
    OBJECT_HEAD;
    MxcValue *field;
} MxcIStruct;

MxcValue new_struct(int);
MxcValue new_error(const char *);

extern const char mxc_36digits[];

#endif
