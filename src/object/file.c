#include <stdio.h>
#include <string.h>
#include "object/mfile.h"
#include "object/strobject.h"
#include "error/error.h"
#include "mem.h"

extern struct mobj_system file_sys;

static MxcValue _new_file(MxcString *path, char *mode) {
  MFile *file = Mxc_malloc(sizeof(MFile));
  FILE *f = fopen(path->str, mode);
  if(!f) {
    /* error */
    return mval_null;
  }
  file->file = f;
  file->path = path;
  SYSTEM(file) = &file_sys;

  return mval_obj(file);
}

static MxcValue new_file_fptr(char *n, FILE *f) {
  MFile *file = Mxc_malloc(sizeof(MFile));
  file->file = f;
  file->path = V2O(new_string_static(n, strlen(n)));
  SYSTEM(file) = &file_sys;

  return mval_obj(file);
}

MxcValue mnew_file(MxcValue *args, size_t nargs) {
  char *mode = nargs == 2? ostr(args[1])->str: "r";

  return _new_file(ostr(args[0]), mode);
}

static MxcValue readline(MFile *f) {
  char buf[1024] = {0};
  if(!fgets(buf, 1024, f->file)) {
    // error
    return mval_null;
  }
  return new_string_copy(buf, strlen(buf));
}

static MxcValue m_readline(MxcValue *args, size_t narg) {
  MFile *f = V2O(args[0]);
  return readline(f);
}

static MxcValue iseof(MFile *f) {
  return feof(f->file)? mval_true: mval_false;
}

static MxcValue m_iseof(MxcValue *args, size_t nargs) {
  MFile *f = V2O(args[0]);
  return iseof(f);
}

void f_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
  MFile *f = (MFile *)ob;
  SYSTEM(f->path)->mark(f->path);
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

struct mobj_system file_sys = {
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
  define_cfunc(mod, "readline", m_readline, mxcty_string, mxcty_file, NULL);
  define_cfunc(mod, "eof", m_iseof, mxcty_bool, mxcty_file, NULL);
  define_cconst(mod, "stdin", new_file_fptr("stdin", stdin), mxcty_file);
  define_cconst(mod, "stdout", new_file_fptr("stdout", stdout), mxcty_file);
  define_cconst(mod, "stderr", new_file_fptr("stderr", stderr), mxcty_file);

  register_module(mod);
}
