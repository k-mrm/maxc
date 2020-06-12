#include <stdio.h>
#include <string.h>
#include "object/mfile.h"
#include "object/strobject.h"
#include "error/error.h"
#include "mem.h"

extern MxcObjImpl file_objimpl;

static MxcValue _new_file(MxcString *path, char *mode) {
  MFile *file = Mxc_malloc(sizeof(MFile));
  FILE *f = fopen(path->str, mode);
  if(!f) {
    /* error */
    return mval_null;
  }
  file->file = f;
  file->path = path;
  OBJIMPL(file) = &file_objimpl;

  return mval_obj(file);
}

static MxcValue new_file_fptr(char *n, FILE *f) {
  MFile *file = Mxc_malloc(sizeof(MFile));
  file->file = f;
  file->path = V2O(new_string_static(n, strlen(n)));
  OBJIMPL(file) = &file_objimpl;

  return mval_obj(file);
}

MxcValue mnew_file(MxcValue *args, size_t nargs) {
  char *mode = nargs == 2? ostr(args[1])->str: "r";

  return _new_file(ostr(args[0]), mode);
}

void f_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
  MFile *f = (MFile *)ob;
  OBJIMPL(f->path)->mark(f->path);
}

void f_guard(MxcObject *ob) {
  ob->gc_guard = 1;
  MFile *f = (MFile *)ob;
  ((MxcObject *)f->path)->gc_guard = 1;
}

void f_unguard(MxcObject *ob) {
  ob->gc_guard = 0;
  MFile *f = (MFile *)ob;
  ((MxcObject *)f->path)->gc_guard = 0;
}

void f_dealloc(MxcObject *s) {
  MFile *f = (MFile *)s;
  fclose(f->file);
  Mxc_free(f->path);
  Mxc_free(f);
}

MxcValue f_tostring(MxcObject *ob) {
  MFile *f = (MFile *)ob;
  return mval_obj(f->path);
}

MxcObjImpl file_objimpl = {
  "File",
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
  MxcModule *mod = new_mxcmodule("File");

  /* File@open */
  define_cfunc(mod, "open", mnew_file, mxcty_file, mxcty_string, NULL);
  define_cfunc(mod, "open", mnew_file, mxcty_file, mxcty_string, mxcty_string, NULL);
  define_cconst(mod, "stdin", new_file_fptr("stdin", stdin), mxcty_file);
  define_cconst(mod, "stdout", new_file_fptr("stdout", stdout), mxcty_file);
  define_cconst(mod, "stderr", new_file_fptr("stderr", stderr), mxcty_file);

  register_module(mod);
}
