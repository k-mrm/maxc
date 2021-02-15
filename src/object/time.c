#include <stdio.h>
#include <time.h>
#include <string.h>
#include "object/object.h"
#include "object/system.h"
#include "object/mtime.h"
#include "object/mstr.h"
#include "mem.h"
#include "mlib.h"

Type *tim_t;

char *asctime_r(const struct tm *tm, char *buf);
struct tm *localtime_r(const time_t *timep, struct tm *result);

MxcValue time_from_utime(time_t utime) {
  struct tm t;
  localtime_r(&utime, &t);

  NEW_OBJECT(MTime, newtime, time_sys);

  newtime->time = t;

  return mval_obj(newtime);
}

MxcValue nowtime() {
  time_t nowsec;
  time_t t = time(&nowsec);
  if(t == (time_t)-1) {
    /* raise error */
    return mval_null;
  }
  
  struct tm nowtm;
  localtime_r(&nowsec, &nowtm);

  NEW_OBJECT(MTime, now, time_sys);

  now->time = nowtm;

  return mval_obj(now);
}

MxcValue m_nowtime(MxcValue *args, size_t na) {
  return nowtime();
}

MxcValue timezone(MTime *t) {
  GC_GUARD(t);

  char buf[16] = {0};
  strftime(buf, 16, "%Z", &t->time);

  MxcValue s = new_string_copy(buf, strlen(buf));

  GC_UNGUARD(t);

  return s;
}

MxcValue m_timezone(MxcValue *args, size_t na) {
  return timezone((MTime *)V2O(args[0]));
}

MxcValue mtime_strftime(MTime *t, MString *fmt) {
  GC_GUARD(t);
  GC_GUARD(fmt);

  char buf[1024] = {0};

  strftime(buf, 1024, fmt->str, &t->time);

  MxcValue s = new_string_copy(buf, strlen(buf));

  GC_UNGUARD(t);
  GC_UNGUARD(fmt);

  return s;
}

MxcValue m_strftime(MxcValue *args, size_t na) {
  return mtime_strftime((MTime *)V2O(args[0]), (MString *)V2O(args[1]));
}

static MxcValue tm_tostring(MxcObject *ob) {
  MTime *t = (MTime *)ob;
  GC_GUARD(t);
  char buf[64] = {0};

  strftime(buf, 64, "%Y-%m-%d %H:%M:%S %z", &t->time);

  MxcValue s = new_string_copy(buf, strlen(buf));

  GC_UNGUARD(t);

  return s;
}

static void tm_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
}

static void tm_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
}

static void tm_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
}

static void tm_dealloc(MxcObject *s) {
  Mxc_free(s);
}

struct mobj_system time_sys = {
  "time",
  NULL,
  tm_tostring,
  tm_dealloc,
  0,
  tm_gc_mark,
  tm_guard,
  tm_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
  0,
  0,
};

void time_init() {
  MxcModule *mod = new_mxcmodule("Time");

  tim_t = userdef_type("Time", T_SHOWABLE);

  define_cfunc(mod, "nowtime", m_nowtime, FTYPE(tim_t));
  define_cfunc(mod, "zone", m_timezone, FTYPE(mxc_string, tim_t));
  define_cfunc(mod, "strftime", m_strftime, FTYPE(mxc_string, tim_t, mxc_string));

  define_ctype(mod, tim_t);

  reg_gmodule(mod);
}
