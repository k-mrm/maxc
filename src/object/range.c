#include <stdio.h>
#include <string.h>
#include "object/mrange.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

extern struct mobj_system range_sys;

static MxcValue new_range(MxcValue b, MxcValue e, int excl) {
  MRange *range = (MRange *)mxc_alloc(sizeof(MRange));
  SYSTEM(range) = &range_sys;
  range->begin = b;
  range->end = e;
  range->excl = excl;

  return mval_obj(range);
}

struct mobj_system range_sys = {
  "range",
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};
