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

#define SYSTEM(ob) (((MxcObject *)ob)->sys)

struct MxcObject {
  struct mobj_system *sys;
  unsigned char marked;
  unsigned char gc_guard;
};

enum valuet {
  VAL_INT     = 0b00000001,
  VAL_FLO     = 0b00000010,
  VAL_TRUE    = 0b00000100,
  VAL_FALSE   = 0b00001000,
  VAL_NULL    = 0b00010000,
  VAL_OBJ     = 0b00100000,
  VAL_INVALID = 0b00000000,
};

struct MxcValue {
  enum valuet t: 8;
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
#define mval_invalid   (MxcValue){ .t = VAL_INVALID, {0}}

#define Invalid_val(v)  (!(v).t)
#define isobj(v)        ((v).t & VAL_OBJ)
#define isint(v)        ((v).t & VAL_INT)
#define isbool(v)       ((v).t & (VAL_TRUE | VAL_FALSE))
#define isflo(v)        ((v).t & VAL_FLO)

#define V2I(v)      ((v).num)
#define V2F(v)      ((v).fnum)
#define V2O(v)      ((v).obj)
#define obig(v)     ((MxcInteger *)(v).obj)
#define ostr(v)     ((MxcString *)(v).obj)
#define ocallee(v)  ((MCallable *)(v).obj)
#define olist(v)    ((MxcList *)(v).obj)
#define ofile(v)    ((MFile *)(v).obj)
#define ostrct(v)   ((MStrct *)(v).obj)

#define mval_debug(v) (ostr(mval2str(v))->str)

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

MxcValue new_struct(int);
MxcValue new_error(const char *);

extern const char mxc_36digits[];

#endif
