/* implementation of string object */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "object/mstr.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue new_string(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ITERABLE(ob)->index = 0;
  ob->str = s;
  ob->isdyn = true;
  ITERABLE(ob)->length = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue new_string_copy(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ob->str = malloc(sizeof(char) * (len + 1));
  memcpy(ob->str, s, len);
  ob->str[len] = '\0';

  ob->isdyn = true;
  ITERABLE(ob)->length = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue new_string_static(char *s, size_t len) {
  MString *ob = (MString *)mxc_alloc(sizeof(MString));
  ob->str = s;
  ob->isdyn = false;
  ITERABLE(ob)->length = len;
  SYSTEM(ob) = &string_sys;

  return mval_obj(ob);
}

MxcValue string_copy(MxcObject *s) {
  MString *n = (MString *)mxc_alloc(sizeof(MString));
  MString *old = (MString *)s;
  *n = *old;

  char *olds = n->str;
  n->str = malloc(sizeof(char) * (ITERABLE(n)->length + 1));
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
  if(str->isdyn) {
    free(str->str);
  }
  Mxc_free(s);
}

MxcValue str_index(MxcIterable *self, MxcValue index) {
  MString *str = (MString *)self;
  int64_t idx = index.num;
  if(self->length <= idx) return mval_invalid;

  return new_char_ref(&str->str[idx]);
}

MxcValue str_index_set(MxcIterable *self, MxcValue index, MxcValue a) {
  MString *str = (MString *)self;
  int64_t idx = index.num;
  if(self->length <= idx) return mval_invalid;
  str->str[idx] = ((MxcChar *)V2O(a))->ch;

  return a;
}

MxcValue str_concat(MString *a, MString *b) {
  size_t len = ITERABLE(a)->length + ITERABLE(b)->length;
  char *res = malloc(sizeof(char) * (len + 1));
  strcpy(res, a->str);
  strcat(res, b->str);

  return new_string(res, len);
}

static MxcValue str_eq(MString *a, MString *b) {
  char *a_cstr = a->str;
  char *b_cstr = b->str;
  return (strcmp(a_cstr, b_cstr) == 0)? mval_true : mval_false;
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

struct mobj_system string_sys = {
  "string",
  string_tostring,
  string_dealloc,
  string_copy,
  string_gc_mark,
  str_guard,
  str_unguard,
  str_index,
  str_index_set,
  0,
  0,
  0,
};
