#include <stdio.h>
#include <string.h>
#include "object/mfile.h"
#include "object/mstr.h"
#include "object/mexception.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

extern struct mobj_system file_sys;

static MxcValue _new_file(MxcString *path, char *mode) {
  MFile *file = (MFile *)mxc_alloc(sizeof(MFile));
  FILE *f = fopen(path->str, mode);
  if(!f) {
    mxc_raise(EXC_NOTFOUND, "No such file: %s", path->str);
    return mval_null;
  }
  file->file = f;
  file->path = path;
  SYSTEM(file) = &file_sys;

  return mval_obj(file);
}

static MxcValue new_file_fptr(char *n, FILE *f) {
  MFile *file = (MFile *)mxc_alloc(sizeof(MFile));
  file->file = f;
  file->path = (MxcString *)V2O(new_string_static(n, strlen(n)));
  SYSTEM(file) = &file_sys;

  return mval_obj(file);
}

MxcValue mnew_file(MContext *f, MxcValue *args, size_t nargs) {
  char *mode = nargs == 2? ostr(args[1])->str: "r";

  return _new_file(ostr(args[0]), mode);
}

static MxcValue readline(MFile *f) {
  char buf[1024] = {0};
  if(!fgets(buf, 1024, f->file)) {
    mxc_raise(EXC_EOF, "End Of File");
    return mval_null;
  }

  return new_string_copy(buf, strlen(buf));
}

static MxcValue m_readline(MContext *f, MxcValue *args, size_t narg) {
  MFile *file = (MFile *)V2O(args[0]);
  return readline(file);
}

static MxcValue writeline(MFile *f, MxcString *s) {
  str_cstr_append(s, "\n", 1);
  if(fputs(s->str, f->file) < 0) {
    mxc_raise(EXC_FILE, "not writable file");
  }
  return mval_null;
}

static MxcValue m_writeline(MContext *f, MxcValue *args, size_t narg) {
  MFile *file = (MFile *)V2O(args[0]);
  MxcString *s = (MxcString *)V2O(args[1]);
  return writeline(file, s);
}

static MxcValue write_core(MFile *f, MxcString *s) {
  if(fputs(s->str, f->file) < 0) {
    mxc_raise(EXC_FILE, "not writable file");
  }
  return mval_null;
}

static MxcValue m_write(MContext *f, MxcValue *args, size_t narg) {
  MFile *file = (MFile *)V2O(args[0]);
  MxcString *s = (MxcString *)V2O(args[1]);
  return write_core(file, s);
}

static MxcValue iseof(MFile *f) {
  int c = fgetc(f->file);
  if(c == EOF) {
    return mval_true;
  }
  ungetc(c, f->file);

  return mval_false;
}

static MxcValue m_iseof(MContext *f, MxcValue *args, size_t nargs) {
  MFile *file = (MFile *)V2O(args[0]);
  return iseof(file);
}

void f_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
  MFile *f = (MFile *)ob;
  SYSTEM(f->path)->mark((MxcObject *)f->path);
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

void flib_init(MInterp *m) {
  MxcModule *mod = new_mxcmodule("File");

  /* File@open */
  define_cfunc(mod, "open", mnew_file, FTYPE(mxcty_file, mxcty_string));
  define_cfunc(mod, "open", mnew_file, FTYPE(mxcty_file, mxcty_string, mxcty_string));
  define_cfunc(mod, "readline", m_readline, FTYPE(mxcty_string, mxcty_file));
  define_cfunc(mod, "writeline", m_writeline, FTYPE(mxcty_none, mxcty_file, mxcty_string));
  define_cfunc(mod, "write", m_write, FTYPE(mxcty_none, mxcty_file, mxcty_string));
  define_cfunc(mod, "eof", m_iseof, FTYPE(mxcty_bool, mxcty_file));
  define_cconst(mod, "stdin", new_file_fptr("stdin", stdin), mxcty_file);
  define_cconst(mod, "stdout", new_file_fptr("stdout", stdout), mxcty_file);
  define_cconst(mod, "stderr", new_file_fptr("stderr", stderr), mxcty_file);

  register_module(m, mod);
}
