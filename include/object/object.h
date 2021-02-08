#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include <stdbool.h>
#include <inttypes.h>

#define OBJECT_HEAD MxcObject base
#define ITERABLE_OBJECT_HEAD MxcIterable base
#define ITERABLE(ob) ((MxcIterable *)(ob))

#define USE_MARK_AND_SWEEP

#define NAN_BOXING

typedef struct MxcValue MxcValue;

struct MContext;
typedef struct MContext MContext;
struct MString;
typedef struct MString MString;

typedef struct MxcObject MxcObject;
typedef struct MxcIterable MxcIterable;

#define NEW_OBJECT(obtype, name, sys) \
  obtype *name; \
  do {  \
    name = (obtype *)mxc_alloc(sizeof(obtype)); \
    SYSTEM(name) = &(sys);  \
  } while(0)

#define SYSTEM(ob) (((MxcObject *)(ob))->sys)

#define GCMARK_FLAG       0b01
#define GCGUARD_FLAG      0b10

#define OBJGCMARK(ob)     ((MxcObject *)(ob))->flag |= GCMARK_FLAG
#define OBJGCUNMARK(ob)   ((MxcObject *)(ob))->flag &= ~GCMARK_FLAG
#define OBJGCMARKED(ob)   (((MxcObject *)(ob))->flag & GCMARK_FLAG)
#define OBJGCGUARD(ob)    ((MxcObject *)(ob))->flag |= GCGUARD_FLAG
#define OBJGCUNGUARD(ob)  ((MxcObject *)(ob))->flag &= ~GCGUARD_FLAG
#define OBJGCGUARDED(ob)  (((MxcObject *)(ob))->flag & GCGUARD_FLAG)

struct mobj_system;
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
struct MxcValue {
  union {
    double d;
    uint64_t raw;
  };
};

enum valuet {
  VAL_INVALID = 0b000,
  VAL_NULL    = 0b100,
  VAL_FALSE   = 0b010,
  VAL_TRUE    = 0b101,
  VAL_INT     = 0b001,
  VAL_OBJ     = 0b011,
  VAL_FLO     = 0b111,
};
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
 *  obj:   1111111111110011 PPPPPPPPPPPPPPPP PPPPPPPPPPPPPPPP PPPPPPPPPPPPPPPP
 *  int:   1111111111110001 0000000000000000 IIIIIIIIIIIIIIII IIIIIIIIIIIIIIII
 */

#ifdef NAN_BOXING

#define MNAN  0xfff0000000000000lu

#define mval_int(i)     (MxcValue){ .raw = (MNAN | ((uint64_t)VAL_INT << 48) | (uint32_t)(i)) }
#define mval_float(f)   (MxcValue){ .d = (f) }
#define mval_obj(p)     (MxcValue){ .raw = (MNAN | ((uint64_t)VAL_OBJ << 48) | (uint64_t)(p)) }
#define mval_true       (MxcValue){ .raw = (MNAN | ((uint64_t)VAL_TRUE << 48) | 1) }
#define mval_false      (MxcValue){ .raw = (MNAN | ((uint64_t)VAL_FALSE << 48)) }
#define mval_null       (MxcValue){ .raw = (MNAN | ((uint64_t)VAL_NULL << 48)) }
#define mval_invalid    (MxcValue){ .raw = MNAN }
#define mval_raw(r)     (MxcValue){ .raw = r }

#define mval_type(v)    ((((v).raw) & MNAN) == MNAN? (((v).raw) >> 48) & 0xf : VAL_FLO)

#define V2I(v)          ((int32_t)(((v).raw) & 0xffffffff))
#define V2O(v)          ((MxcObject *)(((v).raw) & 0xffffffffffff))
#define V2F(v)          ((v).d)
#define obig(v)         ((MInteger *)V2O(v))
#define ostr(v)         ((MString *)V2O(v))
#define ocallee(v)      ((MCallable *)V2O(v))
#define olist(v)        ((MList *)V2O(v))
#define ofile(v)        ((MFile *)V2O(v))
#define ostrct(v)       ((MStrct *)V2O(v))

#define isobj(v)        (mval_type(v) == VAL_OBJ)
#define isint(v)        (mval_type(v) == VAL_INT)
#define isbool(v)       (mval_type(v) == VAL_TRUE || mval_type(v) == VAL_FALSE)
#define istrue(v)       (mval_type(v) == VAL_TRUE)
#define isfalse(v)      (mval_type(v) == VAL_FALSE)
#define isflo(v)        (mval_type(v) == VAL_FLO)

#define check_value(v)  ((v).raw != MNAN)

#define val_raw(v)      ((v).raw)

#else   /* !NAN_BOXING */

#define mval_type(v)   ((v).t)

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
#define obig(v)     ((MInteger *)(v).obj)
#define ostr(v)     ((MString *)(v).obj)
#define ocallee(v)  ((MCallable *)(v).obj)
#define olist(v)    ((MList *)(v).obj)
#define ofile(v)    ((MFile *)(v).obj)
#define ostrct(v)   ((MStrct *)(v).obj)

#endif  /* NAN_BOXING */

#define mval_debug(v) (ostr(mval2str(v))->str)

MxcValue mval2str(MxcValue);
MxcValue mval_copy(MxcValue);
bool mval_eq(MxcValue, MxcValue);
void mgc_mark(MxcValue);
void mgc_guard(MxcValue);
void mgc_unguard(MxcValue);

uint32_t obj_hash32(MxcObject *ob);
uint32_t mval_hash32(MxcValue v);

extern const char mxc_36digits[];

#endif
