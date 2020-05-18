/* implementation of string object */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "object/strobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue new_string(char *s, size_t len) {
  MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
  ITERABLE(ob)->index = 0;
  ITERABLE(ob)->next = mval_invalid;
  ob->str = s;
  ob->isdyn = true;
  ITERABLE(ob)->length = len;
  OBJIMPL(ob) = &string_objimpl; 

  return mval_obj(ob);
}

MxcValue new_string_copy(char *s, size_t len) {
  MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
  ITERABLE(ob)->index = 0;
  ITERABLE(ob)->next = mval_invalid;
  ob->str = malloc(sizeof(char) * (len + 1));
  memcpy(ob->str, s, len);
  ob->str[len] = '\0';

  ob->isdyn = true;
  ITERABLE(ob)->length = len;
  OBJIMPL(ob) = &string_objimpl; 

  return mval_obj(ob);
}

MxcValue new_string_static(char *s, size_t len) {
  MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
  ITERABLE(ob)->index = 0;
  ITERABLE(ob)->next = mval_invalid;
  ob->str = s;
  ob->isdyn = false;
  ITERABLE(ob)->length = len;
  OBJIMPL(ob) = &string_objimpl; 

  return mval_obj(ob);
}

MxcValue string_copy(MxcObject *s) {
  MxcString *n = (MxcString *)Mxc_malloc(sizeof(MxcString));
  MxcString *old = (MxcString *)s;
  *n = *old; 

  char *olds = n->str;
  n->str = malloc(sizeof(char) * (ITERABLE(n)->length + 1));
  strcpy(n->str, olds);
  n->isdyn = true;

  return mval_obj(n);
}

void string_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
}

void str_guard(MxcObject *ob) {
  ob->gc_guard = 1;
}

void str_unguard(MxcObject *ob) {
  ob->gc_guard = 0;
}

void string_dealloc(MxcObject *s) {
  MxcString *str = (MxcString *)s;
  if(str->isdyn) {
    free(str->str);
  }
  Mxc_free(s);
}

MxcValue str_index(MxcIterable *self, int64_t idx) {
  MxcString *str = (MxcString *)self;
  if(self->length <= idx) return mval_invalid;

  return new_char_ref(&str->str[idx]);
}

MxcValue str_index_set(MxcIterable *self, int64_t idx, MxcValue a) {
  MxcString *str = (MxcString *)self;
  if(self->length <= idx) return mval_invalid;
  str->str[idx] = ((MxcChar *)optr(a))->ch;

  return a;
}

MxcValue str_concat(MxcValue a, MxcValue b) {
  size_t len = ITERABLE(ostr(a))->length + ITERABLE(ostr(b))->length;
  char *res = malloc(sizeof(char) * (len + 1));
  strcpy(res, ostr(a)->str);
  strcat(res, ostr(b)->str);

  return new_string(res, len);
}

void str_cstr_append(MxcValue a, char *b, size_t blen) {
  size_t len = ITERABLE(ostr(a))->length + blen;
  if(ostr(a)->isdyn) {
    ostr(a)->str = realloc(ostr(a)->str, sizeof(char) * (len + 1));
  }
  else {
    char *buf = malloc(sizeof(char) * (len + 1));
    strcpy(buf, ostr(a)->str);
    ostr(a)->str = buf;
  }

  strcat(ostr(a)->str, b);
  ITERABLE(ostr(a))->length = len;
  ostr(a)->isdyn = true;
}

void str_append(MxcValue a, MxcValue b) {
  str_cstr_append(a, ostr(b)->str, ITERABLE(ostr(b))->length);
}

MxcValue string_tostring(MxcObject *ob) {
  return mval_obj(ob);
}

MxcObjImpl string_objimpl = {
  "string",
  string_tostring,
  string_dealloc,
  string_copy,
  string_gc_mark,
  str_guard,
  str_unguard,
  str_index,
  str_index_set,
};
