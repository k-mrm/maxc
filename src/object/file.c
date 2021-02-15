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
#include "object/mtime.h"
#include "error/error.h"
#include "mem.h"
#include "mlib.h"

extern struct mobj_system file_sys;

int fileno(FILE *);

MxcValue new_stat(struct stat st);

MxcValue mfstdin;
MxcValue mfstdout;
MxcValue mfstderr;

static MxcValue _new_file(MString *path, char *mode) {
  NEW_OBJECT(MFile, file, file_sys);
  FILE *f = fopen(path->str, mode);
  file->file = f;
  file->path = path;
  if(!f) {
    mxc_raise(EXC_FILE, "%s", strerror(errno));
    fclose(f);
    return mval_null;
  }

  return mval_obj(file);
}

static MxcValue new_file_fptr(char *n, FILE *f) {
  NEW_OBJECT(MFile, file, file_sys);
  file->file = f;
  file->path = (MString *)V2O(new_string_static(n, strlen(n)));

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

static MxcValue file_stat(MFile *f) {
  struct stat st;
  if(fstat(fileno(f->file), &st) < 0) {
    mxc_raise(EXC_FILE, "fstat failed");
    return mval_null;
  }

  return new_stat(st);
}

static MxcValue mfile_stat(MxcValue *args, size_t na) {
  return file_stat((MFile *)V2O(args[0]));
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
  GC_GUARD(f);
  char buf[1024] = {0}; /* ? */
  sprintf(buf, "<File: %s>", f->path->str);

  MxcValue s = new_string_copy(buf, strlen(buf));

  GC_UNGUARD(f);

  return s;
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
  0,
};

MxcValue new_stat(struct stat st) {
  NEW_OBJECT(MStat, s, stat_sys);

  s->st = st;

  return mval_obj(s);
}

static MxcValue stat_ino(MStat *st) {
  return mval_int(st->st.st_ino);
}

static MxcValue mstat_ino(MxcValue *args, size_t na) {
  return stat_ino((MStat *)V2O(args[0]));
}

static MxcValue stat_dev(MStat *st) {
  return mval_int(st->st.st_dev);
}

static MxcValue mstat_dev(MxcValue *args, size_t na) {
  return stat_dev((MStat *)V2O(args[0]));
}

static MxcValue stat_uid(MStat *st) {
  return mval_int(st->st.st_uid);
}

static MxcValue mstat_uid(MxcValue *args, size_t na) {
  return stat_uid((MStat *)V2O(args[0]));
}

static MxcValue stat_gid(MStat *st) {
  return mval_int(st->st.st_gid);
}

static MxcValue mstat_gid(MxcValue *args, size_t na) {
  return stat_gid((MStat *)V2O(args[0]));
}

static MxcValue stat_size(MStat *st) {
  return mval_int(st->st.st_size);
}

static MxcValue mstat_size(MxcValue *args, size_t na) {
  return stat_size((MStat *)V2O(args[0]));
}

static MxcValue stat_mode(MStat *st) {
  return mval_int(st->st.st_mode);
}

static MxcValue mstat_mode(MxcValue *args, size_t na) {
  return stat_mode((MStat *)V2O(args[0]));
}

static MxcValue stat_nlink(MStat *st) {
  return mval_int(st->st.st_nlink);
}

static MxcValue mstat_nlink(MxcValue *args, size_t na) {
  return stat_nlink((MStat *)V2O(args[0]));
}

static MxcValue stat_atime(MStat *st) {
  return time_from_utime(st->st.st_atime);
}

static MxcValue mstat_atime(MxcValue *args, size_t na) {
  return stat_atime((MStat *)V2O(args[0]));
}

static MxcValue stat_mtime(MStat *st) {
  return time_from_utime(st->st.st_mtime);
}

static MxcValue mstat_mtime(MxcValue *args, size_t na) {
  return stat_mtime((MStat *)V2O(args[0]));
}

static MxcValue stat_ctime(MStat *st) {
  return time_from_utime(st->st.st_ctime);
}

static MxcValue mstat_ctime(MxcValue *args, size_t na) {
  return stat_ctime((MStat *)V2O(args[0]));
}

static MxcValue stat_isfile(MStat *st) {
  return mval_bool(S_ISREG(st->st.st_mode));
}

static MxcValue mstat_isfile(MxcValue *args, size_t na) {
  return stat_isfile((MStat *)V2O(args[0]));
}

static MxcValue stat_isdir(MStat *st) {
  return mval_bool(S_ISDIR(st->st.st_mode));
}

static MxcValue mstat_isdir(MxcValue *args, size_t na) {
  return stat_isdir((MStat *)V2O(args[0]));
}

static MxcValue stat_ischr(MStat *st) {
  return mval_bool(S_ISCHR(st->st.st_mode));
}

static MxcValue mstat_ischr(MxcValue *args, size_t na) {
  return stat_ischr((MStat *)V2O(args[0]));
}

static MxcValue stat_isblk(MStat *st) {
  return mval_bool(S_ISBLK(st->st.st_mode));
}

static MxcValue mstat_isblk(MxcValue *args, size_t na) {
  return stat_isblk((MStat *)V2O(args[0]));
}

static MxcValue stat_isfifo(MStat *st) {
  return mval_bool(S_ISFIFO(st->st.st_mode));
}

static MxcValue mstat_isfifo(MxcValue *args, size_t na) {
  return stat_isfifo((MStat *)V2O(args[0]));
}

static MxcValue stat_islnk(MStat *st) {
  return mval_bool(S_ISLNK(st->st.st_mode));
}

static MxcValue mstat_islnk(MxcValue *args, size_t na) {
  return stat_islnk((MStat *)V2O(args[0]));
}

void st_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
}

void st_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
}

void st_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
}

void st_dealloc(MxcObject *s) {
  Mxc_free(s);
}

MxcValue st_tostring(MxcObject *ob) {
  MStat *st = (MStat *)ob;
  GC_GUARD(st);

  /* FIXME */
  MxcValue s = new_string_static("stat", strlen("stat"));

  GC_UNGUARD(st);

  return s;
}

struct mobj_system stat_sys = {
  "stat",
  NULL,
  st_tostring,
  st_dealloc,
  0,
  st_gc_mark,
  st_guard,
  st_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
  0,
  0,
};

extern Type *tim_t;

void file_init() {
  time_init();

  MxcModule *mod = new_mxcmodule("File");

  mfstdin = new_file_fptr("stdin", stdin);
  mfstdout = new_file_fptr("stdout", stdout);
  mfstderr = new_file_fptr("stderr", stderr);

  Type *file_t = userdef_type("File", T_SHOWABLE);
  Type *stat_t = userdef_type("Stat", T_SHOWABLE);

  define_cfunc(mod, "open", mnew_file, FTYPE(file_t, mxc_string));
  define_cfunc(mod, "open", mnew_file, FTYPE(file_t, mxc_string, mxc_string));
  define_cfunc(mod, "readline", m_readline, FTYPE(mxc_string, file_t));
  define_cfunc(mod, "readline", m_readline, FTYPE(mxc_string));
  define_cfunc(mod, "read", mfread, FTYPE(mxc_string, file_t));
  define_cfunc(mod, "writeline", m_writeline, FTYPE(mxc_none, file_t, mxc_string));
  define_cfunc(mod, "write", m_write, FTYPE(mxc_none, file_t, mxc_string));
  define_cfunc(mod, "eof", m_iseof, FTYPE(mxc_bool, file_t));
  define_cfunc(mod, "rewind", m_frewind, FTYPE(mxc_none, file_t));
  define_cfunc(mod, "close", mfclose, FTYPE(mxc_none, file_t));
  define_cfunc(mod, "size", mfsize, FTYPE(mxc_int, file_t));
  define_cfunc(mod, "stat", mfile_stat, FTYPE(stat_t, file_t));
  define_cconst(mod, "stdin", mfstdin, file_t);
  define_cconst(mod, "stdout", mfstdout, file_t);
  define_cconst(mod, "stderr", mfstderr, file_t);

  define_cfunc(mod, "ino", mstat_ino, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "dev", mstat_dev, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "uid", mstat_uid, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "gid", mstat_gid, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "size", mstat_size, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "mode", mstat_mode, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "nlink", mstat_nlink, FTYPE(mxc_int, stat_t));
  define_cfunc(mod, "atime", mstat_atime, FTYPE(tim_t, stat_t));
  define_cfunc(mod, "mtime", mstat_mtime, FTYPE(tim_t, stat_t));
  define_cfunc(mod, "ctime", mstat_ctime, FTYPE(tim_t, stat_t));
  define_cfunc(mod, "isfile", mstat_isfile, FTYPE(mxc_bool, stat_t));
  define_cfunc(mod, "isdir", mstat_isdir, FTYPE(mxc_bool, stat_t));
  define_cfunc(mod, "ischr", mstat_ischr, FTYPE(mxc_bool, stat_t));
  define_cfunc(mod, "isblk", mstat_isblk, FTYPE(mxc_bool, stat_t));
  define_cfunc(mod, "isfifo", mstat_isfifo, FTYPE(mxc_bool, stat_t));
  define_cfunc(mod, "islnk", mstat_islnk, FTYPE(mxc_bool, stat_t));

  define_ctype(mod, file_t);
  define_ctype(mod, stat_t);

  reg_gmodule(mod);
}

