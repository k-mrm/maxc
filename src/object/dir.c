#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "object/mdir.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

static MxcValue newdir(MString *path) {
  MDir *md = (MDir *)mxc_alloc(sizeof(MDir));

  DIR *dir = opendir(path->str);
  if(!dir) {
    /* raise error */
    return mval_null;
  }

  md->dir = dir;
  md->path = path;
  SYSTEM(md) = &dir_sys;

  return mval_obj(md);
}

MxcValue opendir(MxcValue *args, size_t nargs) {
  return newdir(ostr(args[0]));
}

static MxcValue dir_read(MDir *dir) {
  ;
}

static void d_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
  MDir *d = (MDir *)ob;
  SYSTEM(d->path)->mark((MxcObject *)d->path);
}

static void d_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
  MDir *d = (MDir *)ob;
  SYSTEM(d->path)->guard((MxcObject *)d->path);
}

static void d_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
  MDir *d = (MDir *)ob;
  SYSTEM(d->path)->unguard((MxcObject *)d->path);
}

void d_dealloc(MxcObject *s) {
  MDir *d = (MDir *)s;
  closedir(d->dir);
  Mxc_free(d->path);
  Mxc_free(d);
}

MxcValue d_tostring(MxcObject *ob) {
  MDir *d = (MDir *)ob;
  GC_GUARD(d);
  char buf[1024] = {0}; /* ? */
  sprintf(buf, "<Dir: %s>", d->path->str);

  MxcValue s = new_string(buf, strlen(buf));

  GC_UNGUARD(d);

  return s;
}

struct mobj_system dir_sys = {
  "Dir",
  NULL,
  d_tostring,
  d_dealloc,
  0,
  d_gc_mark,
  d_guard,
  d_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
  0,
};

MxcModule *dirlib_module() {
  MxcModule *mod = new_mxcmodule("Dir");

  Type *dir_t = userdef_type("Dir", T_SHOWABLE);

  define_cfunc(mod, "opendir", opendir, FTYPE(dir_t, mxc_string));

  return mod;
}

