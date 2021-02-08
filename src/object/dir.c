#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "object/object.h"
#include "object/mdir.h"
#include "object/system.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

static MxcValue newdir(MString *path) {
  NEW_OBJECT(MDir, md, dir_sys);

  DIR *dir = opendir(path->str);
  if(!dir) {
    /* raise error */
    return mval_null;
  }

  md->dir = dir;
  md->path = path;

  return mval_obj(md);
}

MxcValue m_opendir(MxcValue *args, size_t nargs) {
  return newdir(ostr(args[0]));
}

static MxcValue dir_read(MDir *dir) {
  ;
}

static MxcValue dir_children(MDir *dir) {
  struct dirent *de;
  MList *l = (MList *)V2O(new_list(4));
  OBJGCGUARD(dir);
  OBJGCGUARD(l);

  while((de = readdir(dir->dir))) {
    char *fname = de->d_name;
    if(!strcmp(fname, ".") || !strcmp(fname, "..")) {
      continue;
    }

    listadd(l, new_string_static(fname, strlen(fname)));
  }

  OBJGCUNGUARD(dir);
  OBJGCUNGUARD(l);

  return mval_obj(l);
}

static MxcValue mdir_children(MxcValue *args, size_t nargs) {
  return dir_children((MDir *)V2O(args[0]));
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

void dir_init() {
  MxcModule *mod = new_mxcmodule("Dir");

  Type *dir_t = userdef_type("Dir", T_SHOWABLE);

  define_cfunc(mod, "opendir", m_opendir, FTYPE(dir_t, mxc_string));
  define_cfunc(mod, "children", mdir_children, FTYPE(new_type_list(mxc_string), dir_t));

  define_ctype(mod, dir_t);

  reg_gmodule(mod);
}

