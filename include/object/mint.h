#ifndef MXC_INTOBJECT_H
#define MXC_INTOBJECT_H

#include <limits.h>
#include "object/object.h"
#include "object/minteger.h"

struct MxcBool;
typedef struct MxcBool MxcBool;

void int_divrem(MxcValue, MxcValue, MxcValue *, MxcValue *);
MxcValue int_tostring(MxcValue);

#define mul_overflow_chk(x, y)  \
    ((x) == 0 ? 0 :  \
     (x) == -1 ? (y) < -(INT64_MAX) : \
     (x) > 0 ?  \
        ((y) > 0 ? INT64_MAX / (x) < (y) : INT64_MIN / (x) > (y)) :  \
        ((y) > 0 ? INT64_MIN / (x) < (y) : INT64_MAX / (x) > (y)))

static inline MxcValue int_copy(MxcValue v) {
  return v;
}

static inline MxcValue int_add(MxcValue l, MxcValue r) {
  int64_t res;
  if(__builtin_add_overflow(l.num, r.num, &res)) {
    l = int_to_integer(l.num);
    r = int_to_integer(r.num);
    return integer_add(l, r); 
  }
  return mval_int(res);
}

static inline MxcValue int_sub(MxcValue l, MxcValue r) {
  int64_t res;
  if(__builtin_sub_overflow(l.num, r.num, &res)) {
    l = int_to_integer(l.num);
    r = int_to_integer(r.num);
    return integer_sub(l, r); 
  }
  return mval_int(res);
}

static inline MxcValue int_mul(MxcValue l, MxcValue r) {
  if(mul_overflow_chk(l.num, r.num)) {
    l = int_to_integer(l.num);
    r = int_to_integer(r.num);
    return integer_mul(l, r); 
  }
  return mval_int(l.num * r.num);
}

static inline MxcValue int_div(MxcValue l, MxcValue r) {
  MxcValue quo;
  int_divrem(l, r, &quo, NULL);
  return quo;
}

static inline MxcValue int_rem(MxcValue l, MxcValue r) {
  MxcValue rem;
  int_divrem(l, r, NULL, &rem);
  return rem;
}

static inline MxcValue int_eq(MxcValue l, MxcValue r) {
  if(l.num == r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue int_noteq(MxcValue l, MxcValue r) {
  if(l.num != r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue int_lt(MxcValue l, MxcValue r) {
  if(l.num < r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue int_lte(MxcValue l, MxcValue r) {
  if(l.num <= r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue int_gt(MxcValue l, MxcValue r) {
  if(l.num > r.num)
    return mval_true;
  else
    return mval_false;
}

static inline MxcValue int_gte(MxcValue l, MxcValue r) {
  if(l.num >= r.num)
    return mval_true;
  else
    return mval_false;
}

#define IntAdd(l, r) (mval_int((l).num + (r).num))
#define IntSub(l, r) (mval_int((l).num - (r).num))
#define IntMul(l, r) (mval_int((l).num * (r).num))
#define IntDiv(l, r) (mval_int((l).num / (r).num))
#define IntXor(l, r) (mval_int((l).num ^ (r).num))

#endif
