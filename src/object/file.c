#include <stdio.h>
#include "object/mfile.h"
#include "error/error.h"
#include "mem.h"

extern MxcObjImpl file_objimpl;

static MxcValue _new_file(MxcString *name, char *mode) {
  MFile *file = Mxc_malloc(sizeof(MFile));
  FILE *f = fopen(name->str, mode);
  if(!f) {
    /* error */
    return mval_null;
  }
  file->file = f;
  file->fname = name;
  OBJIMPL(file) = &file_objimpl;

  return mval_obj(file);
}

MxcValue mnew_file(MxcValue *args, size_t nargs) {
  char *mode = nargs == 2? ostr(args[1]): "r";

  return _new_file(ostr(args[0]), mode);
}

void f_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
  MFile *f = (MFile *)ob;
  OBJIMPL(f->fname)->mark(f->fname);
}

void f_guard(MxcObject *ob) {
  ob->gc_guard = 1;
  MFile *f = (MFile *)ob;
  ((MxcObject *)f->fname)->gc_guard = 1;
}

void f_unguard(MxcObject *ob) {
  ob->gc_guard = 0;
  MFile *f = (MFile *)ob;
  ((MxcObject *)f->fname)->gc_guard = 0;
}

void f_dealloc(MxcObject *s) {
  MFile *f = (MFile *)s;
  fclose(f->file);
  Mxc_free(f->fname);
  Mxc_free(f);
}

MxcValue f_tostring(MxcObject *ob) {
  MFile *f = (MFile *)ob;
  return mval_obj(f->fname);
}

MxcObjImpl file_objimpl = {
  "file",
  f_tostring,
  f_dealloc,
  0,
  f_gc_mark,
  f_guard,
  f_unguard,
  0,
  0,
};


void flib_init() {
  MxcModule *mod = new_mxcmodule("file");

  register_module(mod);
}
