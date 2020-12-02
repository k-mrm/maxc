#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include <inttypes.h>
#include "object/system.h"

#define OBJECT_HEAD MxcObject base
#define ITERABLE_OBJECT_HEAD MxcIterable base
#define ITERABLE(ob) ((MxcIterable *)(ob))

#define USE_MARK_AND_SWEEP

// #define NAN_BOXING

struct MContext;
typedef struct MContext MContext;
struct MString;
typedef struct MString MString;

typedef struct MxcObject MxcObject;
typedef struct MxcIterable MxcIterable;

#define SYSTEM(ob) (((MxcObject *)(ob))->sys)

#define GCMARK_FLAG       0b01
#define GCGUARD_FLAG      0b10

#define OBJGCMARK(ob)     ((MxcObject *)(ob))->flag |= GCMARK_FLAG
#define OBJGCUNMARK(ob)   ((MxcObject *)(ob))->flag &= ~GCMARK_FLAG
#define OBJGCMARKED(ob)   (((MxcObject *)(ob))->flag & GCMARK_FLAG)
#define OBJGCGUARD(ob)    ((MxcObject *)(ob))->flag |= GCGUARD_FLAG
#define OBJGCUNGUARD(ob)  ((MxcObject *)(ob))->flag &= ~GCGUARD_FLAG
#define OBJGCGUARDED(ob)  (((MxcObject *)(ob))->flag & GCGUARD_FLAG)

struct MxcObject {
  struct mobj_system *sys;
  /*
   *  8bit flag
   *  rrrrrrgm
   *  
   *  r: reserved
   *  g: gc guard
   *  m: gc marked
   */
  uint8_t flag;
};

#ifdef NAN_BOXING
typedef uint64_t MxcValue;
#else   /* NAN_BOXING */
enum valuet {
  VAL_INT     = 0b00000001,
  VAL_FLO     = 0b00000010,
  VAL_TRUE    = 0b00000100,
  VAL_FALSE   = 0b00001000,
  VAL_NULL    = 0b00010000,
  VAL_OBJ     = 0b00100000,
  VAL_INVALID = 0b00000000,
};

typedef struct MxcValue MxcValue;
struct MxcValue {
  enum valuet t: 8;
  union {
    MxcObject *obj;
    int64_t num;
    double fnum;
  };
};
#endif  /* NAN_BOXING */

/*
 *  float: FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFF
 *  obj:   1111111111110000 PPPPPPPPPPPPPPPP PPPPPPPPPPPPPPPP PPPPPPPPPPPPPPPP
 *  int:   1111111111110001 IIIIIIIIIIIIIIII IIIIIIIIIIIIIIII IIIIIIIIIIIIIIII(?)
 */

#ifdef NAN_BOXING

#define MNAN  0xfff0_0000_0000_0000
#define mval_float(v)  (v)

#define V2O(v) (MxcObject *)((v) & 0xffffffffffff)
#define V2F(v) (v)

#else   /* NAN_BOXING */

#define mval_int(v)    (MxcValue){ .t = VAL_INT, .num = (v) }
#define mval_float(v)  (MxcValue){ .t = VAL_FLO, .fnum = (v) }
#define mval_true      (MxcValue){ .t = VAL_TRUE, .num = 1 }
#define mval_false     (MxcValue){ .t = VAL_FALSE, .num = 0 }
#define mval_null      (MxcValue){ .t = VAL_NULL, .num = 0 }
#define mval_obj(v)    (MxcValue){ .t = VAL_OBJ, .obj = (MxcObject *)(v) }
#define mval_invalid   (MxcValue){ .t = VAL_INVALID, {0}}

#define check_value(v)  ((v).t)
#define isobj(v)        ((v).t & VAL_OBJ)
#define isint(v)        ((v).t & VAL_INT)
#define isbool(v)       ((v).t & (VAL_TRUE | VAL_FALSE))
#define istrue(v)       ((v).t == VAL_TRUE)
#define isfalse(v)      ((v).t == VAL_FALSE)
#define isflo(v)        ((v).t & VAL_FLO)

#define V2I(v)      ((v).num)
#define V2F(v)      ((v).fnum)
#define V2O(v)      ((v).obj)
#define obig(v)     ((MxcInteger *)(v).obj)
#define ostr(v)     ((MString *)(v).obj)
#define ocallee(v)  ((MCallable *)(v).obj)
#define olist(v)    ((MList *)(v).obj)
#define ofile(v)    ((MFile *)(v).obj)
#define ostrct(v)   ((MStrct *)(v).obj)

#endif  /* NAN_BOXING */

#define mval_debug(v) (ostr(mval2str(v))->str)

MxcValue mval2str(MxcValue);
MxcValue mval_copy(MxcValue);
void mgc_mark(MxcValue);
void mgc_guard(MxcValue);
void mgc_unguard(MxcValue);

extern const char mxc_36digits[];

#endif
