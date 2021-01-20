/* implementation of string object */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "object/mstr.h"
#include "object/system.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"
#include "mlib.h"

MxcValue new_string(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ITERABLE(ob)->index = 0;
  ob->str = s;
  ob->isdyn = true;
  STRLEN(ob) = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue new_string_copy(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ob->str = malloc(sizeof(char) * (len + 1));
  memcpy(ob->str, s, len);
  ob->str[len] = '\0';

  ob->isdyn = true;
  STRLEN(ob) = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue new_string_static(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ob->str = s;
  ob->isdyn = false;
  STRLEN(ob) = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue string_copy(MxcObject *s) {
  MString *n = (MString *)mxc_alloc(sizeof(MString));
  MString *old = (MString *)s;
  *n = *old;

  char *olds = n->str;
  n->str = malloc(sizeof(char) * (STRLEN(n) + 1));
  strcpy(n->str, olds);
  n->isdyn = true;

  return mval_obj(n);
}

static void string_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
}

static void str_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
}

static void str_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
}

static void string_dealloc(MxcObject *s) {
  MString *str = (MString *)s;
  if(str->isdyn)
    free(str->str);
  Mxc_free(s);
}

MxcValue str_index(MxcIterable *self, MxcValue index) {
  MString *str = (MString *)self;
  int32_t idx = V2I(index);
  if(idx < 0) {
    idx = STRLEN(str) + idx;
  }
  if(STRLEN(str) <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }

  return new_string_copy(&str->str[idx], 1);
}

MxcValue str_index_set(MxcIterable *self, MxcValue index, MxcValue a) {
  MString *str = (MString *)self;
  MString *src = (MString *)V2O(a);
  int32_t idx = V2I(index);
  if(idx < 0) {
    idx = STRLEN(str) + idx;
  }
  if(STRLEN(str) <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }

  if(STRLEN(src) == 0) {
    // TODO;
  }
  else if(STRLEN(src) == 1) {
    str->str[idx] = src->str[0];
  }
  else {
    // TODO;
  }

  return a;
}

MxcValue str_concat(MString *a, MString *b) {
  size_t len = STRLEN(a) + STRLEN(b);
  char *res = malloc(sizeof(char) * (len + 1));
  strcpy(res, a->str);
  strcat(res, b->str);

  return new_string(res, len);
}

static MxcValue strupcase(MString *s) {
  MString *v = (MString *)V2O(new_string_copy(s->str, STRLEN(s)));
  for(int i = 0; i < STRLEN(v); i++) {
    char c = v->str[i];
    if('a' <= c && c <= 'z') {
      c -= 0x20;
    }
    v->str[i] = c;
  }

  return mval_obj(v);
}

static MxcValue mstrupcase(MxcValue *a, size_t na) {
  return strupcase((MString *)V2O(a[0]));
}

static MxcValue strdowncase(MString *s) {
  MString *v = (MString *)V2O(new_string_copy(s->str, STRLEN(s)));
  for(int i = 0; i < STRLEN(v); i++) {
    char c = v->str[i];
    if('A' <= c && c <= 'Z') {
      c += 0x20;
    }
    v->str[i] = c;
  }

  return mval_obj(v);
}

static MxcValue mstrdowncase(MxcValue *a, size_t na) {
  return strdowncase((MString *)V2O(a[0]));
}

static MxcValue strsplit(MString *s, MString *d) {
  char tmp[STRLEN(s) + 1];
  memset(tmp, 0, STRLEN(s) + 1);
  strcpy(tmp, s->str);
  char *p = strtok(tmp, d->str);
  MxcValue vl = new_list(4);
  MList *l = (MList *)V2O(vl);
  listadd(l, new_string_copy(p, strlen(p)));

  while(1) {
    p = strtok(NULL, d->str);
    if(p) {
      listadd(l, new_string_copy(p, strlen(p)));
    }
    else {
      break;
    }
  }

  return vl;
}

static MxcValue mstrsplit(MxcValue *a, size_t na) {
  return strsplit((MString *)V2O(a[0]), (MString *)V2O(a[1]));
}

static MxcValue strempty(MString *s) {
  if(STRLEN(s) == 0) {
    return mval_true;
  }
  else {
    return mval_false;
  }
}

static MxcValue mstrempty(MxcValue *a, size_t na) {
  return strempty((MString *)V2O(a[0]));
}

static MxcValue strclear(MString *s) {
  if(s->isdyn) {
    free(s->str);
  }
  s->isdyn = false;
  s->str = "";
  STRLEN(s) = 0;
  return mval_null;
}

static MxcValue mstrclear(MxcValue *a, size_t na) {
  return strclear((MString *)V2O(a[0]));
}

static bool internal_str_eq(MxcObject *a, MxcObject *b) {
  char *a_cstr = ((MString *)a)->str;
  char *b_cstr = ((MString *)b)->str;
  return strcmp(a_cstr, b_cstr) == 0;
}

static MxcValue str_eq(MString *a, MString *b) {
  return internal_str_eq(a, b)? mval_true : mval_false;
}

MxcValue mstr_eq(MxcValue *a, size_t na) {
  return str_eq((MString *)V2O(a[0]), (MString *)V2O(a[1]));
}

void str_cstr_append(MString *a, char *b, size_t blen) {
  size_t len = ITERABLE(a)->length + blen;
  if(a->isdyn) {
    a->str = realloc(a->str, sizeof(char) * (len + 1));
  }
  else {
    char *buf = malloc(sizeof(char) * (len + 1));
    strcpy(buf, a->str);
    a->str = buf;
  }

  strcat(a->str, b);
  ITERABLE(a)->length = len;
  a->isdyn = true;
}

void str_append(MString *a, MString *b) {
  str_cstr_append(a, b->str, ITERABLE(b)->length);
}

MxcValue string_tostring(MxcObject *ob) {
  return mval_obj(ob);
}

static MxcValue mstrlen(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MString *ob = ostr(sp[0]);
  int len = ITERABLE(ob)->length;
  return mval_int(len);
}

/* FNV-1 algorithm */
static uint32_t str_hash32(MxcObject *ob) {
  MString *s = (MString *)ob;
  char *key = s->str;
  size_t len = STRLEN(s);

  uint32_t hash = 2166136261;

  for(size_t i = 0; i < len; i++) {
    hash = (hash ^ key[i]) * 16777619;
  }

  return hash;
}

struct mobj_system string_sys = {
  "string",
  NULL,
  string_tostring,
  string_dealloc,
  string_copy,
  string_gc_mark,
  str_guard,
  str_unguard,
  str_index,
  str_index_set,
  iterable_reset,
  iterable_next,
  iterable_stopped,
  str_hash32,
  internal_str_eq,
};

MxcModule *strlib_module() {
  MxcModule *mod = new_mxcmodule("string");

  define_cfunc(mod, "eq", mstr_eq, FTYPE(mxc_bool, mxc_string, mxc_string));
  define_cfunc(mod, "len", mstrlen, FTYPE(mxc_int, mxc_string));
  define_cfunc(mod, "clear", mstrclear, FTYPE(mxc_none, mxc_string));
  define_cfunc(mod, "empty", mstrempty, FTYPE(mxc_bool, mxc_string));
  define_cfunc(mod, "upcase", mstrupcase, FTYPE(mxc_string, mxc_string));
  define_cfunc(mod, "downcase", mstrdowncase, FTYPE(mxc_string, mxc_string));
  Type *slist = new_type_list(mxc_string);
  define_cfunc(mod, "split", mstrsplit, FTYPE(slist, mxc_string, mxc_string));

  return mod;
}
