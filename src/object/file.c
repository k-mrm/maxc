#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "object/mfile.h"
#include "object/system.h"
#include "object/mstr.h"
#include "object/mexception.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

extern struct mobj_system file_sys;

int fileno(FILE *);

MxcValue mfstdin;
MxcValue mfstdout;
MxcValue mfstderr;

static MxcValue _new_file(MString *path, char *mode) {
  MFile *file = (MFile *)mxc_alloc(sizeof(MFile));
  FILE *f = fopen(path->str, mode);
  file->file = f;
  file->path = path;
  SYSTEM(file) = &file_sys;
  if(!f) {
    mxc_raise(EXC_FILE, "%s", strerror(errno));
    fclose(f);
    return mval_null;
  }

  return mval_obj(file);
}

static MxcValue new_file_fptr(char *n, FILE *f) {
  MFile *file = (MFile *)mxc_alloc(sizeof(MFile));
  file->file = f;
  file->path = (MString *)V2O(new_string_static(n, strlen(n)));
  SYSTEM(file) = &file_sys;

  return mval_obj(file);
}

MxcValue mnew_file(MxcValue *args, size_t nargs) {
  char *mode = nargs == 2? ostr(args[1])->str: "r";

  return _new_file(ostr(args[0]), mode);
}

MxcValue fileclose(MFile *f) {
  int e = fclose(f->file);
  if(e) {
    mxc_raise(EXC_FILE, "failed to close file");
  }
  return mval_null;
}

static MxcValue mfclose(MxcValue *a, size_t na) {
  return fileclose((MFile *)V2O(a[0]));
}

static MxcValue fileread(MFile *f) {
  struct stat st;
  if(fstat(fileno(f->file), &st) < 0) {
    mxc_raise(EXC_FILE, "fstat failed");
    return mval_null;
  }
  off_t fsize = st.st_size;

  char *buf = malloc(sizeof(char) * fsize);
  if(fread(buf, sizeof(char), fsize, f->file) < fsize) {
    if(ferror(f->file)) {
      mxc_raise(EXC_FILE, "invalid read");
      clearerr(f->file);
      return mval_null;
    }
  }

  return new_string(buf, fsize);
}

static MxcValue mfread(MxcValue *a, size_t na) {
  return fileread((MFile *)V2O(a[0]));
}

static MxcValue fsize(MFile *f) {
  struct stat st;
  if(fstat(fileno(f->file), &st) < 0) {
    mxc_raise(EXC_FILE, "fstat failed");
    return mval_null;
  }
  return mval_int(st.st_size);
}

static MxcValue mfsize(MxcValue *a, size_t na) {
  return fsize((MFile *)V2O(a[0]));
}

static MxcValue readline(MFile *f) {
  char buf[1024] = {0};
  if(!fgets(buf, 1024, f->file)) {
    mxc_raise(EXC_EOF, "End Of File");
    return mval_null;
  }

  return new_string_copy(buf, strlen(buf));
}

static MxcValue m_readline(MxcValue *args, size_t narg) {
  MFile *file;
  if(narg == 1) {
    file = (MFile *)V2O(args[0]);
  }
  else {
    file = (MFile *)V2O(mfstdin);
  }

  return readline(file);
}

static MxcValue writeline(MFile *f, MString *s) {
  str_cstr_append(s, "\n", 1);
  if(fputs(s->str, f->file) < 0) {
    mxc_raise(EXC_FILE, "not writable file");
  }
  return mval_null;
}

static MxcValue m_writeline(MxcValue *args, size_t narg) {
  MFile *file = (MFile *)V2O(args[0]);
  MString *s = (MString *)V2O(args[1]);
  return writeline(file, s);
}

static MxcValue write_core(MFile *f, MString *s) {
  if(fputs(s->str, f->file) < 0) {
    mxc_raise(EXC_FILE, "not writable file");
  }
  return mval_null;
}

static MxcValue m_write(MxcValue *args, size_t narg) {
  MFile *file = (MFile *)V2O(args[0]);
  MString *s = (MString *)V2O(args[1]);
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

static MxcValue m_iseof(MxcValue *args, size_t nargs) {
  MFile *file = (MFile *)V2O(args[0]);
  return iseof(file);
}

static MxcValue frewind(MFile *f) {
  fseek(f->file, 0, SEEK_SET);
  return mval_null;
}

static MxcValue m_frewind(MxcValue *args, size_t nargs) {
  MFile *file = (MFile *)V2O(args[0]);
  return frewind(file);
}

void f_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
  MFile *f = (MFile *)ob;
  SYSTEM(f->path)->mark((MxcObject *)f->path);
}

void f_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
  MFile *f = (MFile *)ob;
  SYSTEM(f->path)->guard((MxcObject *)f->path);
}

void f_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
  MFile *f = (MFile *)ob;
  SYSTEM(f->path)->unguard((MxcObject *)f->path);
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
  NULL,
  f_tostring,
  f_dealloc,
  0,
  f_gc_mark,
  f_guard,
  f_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
};

MxcModule *flib_module() {
  MxcModule *mod = new_mxcmodule("File");

  mfstdin = new_file_fptr("stdin", stdin);
  mfstdout = new_file_fptr("stdout", stdout);
  mfstderr = new_file_fptr("stderr", stderr);

  /* File@open */
  define_cfunc(mod, "open", mnew_file, FTYPE(mxc_file, mxc_string));
  define_cfunc(mod, "open", mnew_file, FTYPE(mxc_file, mxc_string, mxc_string));
  define_cfunc(mod, "readline", m_readline, FTYPE(mxc_string, mxc_file));
  define_cfunc(mod, "readline", m_readline, FTYPE(mxc_string));
  define_cfunc(mod, "read", mfread, FTYPE(mxc_string, mxc_file));
  define_cfunc(mod, "writeline", m_writeline, FTYPE(mxc_none, mxc_file, mxc_string));
  define_cfunc(mod, "write", m_write, FTYPE(mxc_none, mxc_file, mxc_string));
  define_cfunc(mod, "eof", m_iseof, FTYPE(mxc_bool, mxc_file));
  define_cfunc(mod, "rewind", m_frewind, FTYPE(mxc_none, mxc_file));
  define_cfunc(mod, "close", mfclose, FTYPE(mxc_none, mxc_file));
  define_cfunc(mod, "size", mfsize, FTYPE(mxc_int, mxc_file));
  define_cconst(mod, "stdin", mfstdin, mxc_file);
  define_cconst(mod, "stdout", mfstdout, mxc_file);
  define_cconst(mod, "stderr", mfstderr, mxc_file);

  return mod;
}
