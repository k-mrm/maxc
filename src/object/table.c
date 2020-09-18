#include <stdint.h>
#include <stdlib.h>
#include "object/mtable.h"
#include "mem.h"

#define roundup(n, m) (n + (n % m? m - n % m : 0))

/* FNV-1 algorithm */
static uint32_t hash_32(char *key, size_t len) {
  uint32_t hash = 2166136261;
  
  for(size_t i = 0; i < len; i++) {
    hash = (hash ^ key[i]) * 16777619;
  }

  return hash;
}

static struct mentry *new_entry(MxcValue k, MxcValue v) {
  struct mentry *e = malloc(sizeof(struct mentry));
  e->key = k;
  e->val = v;
  return e;
}

static MxcValue search_val(MTable *t, uint32_t hash) {
}

MxcValue new_table(MxcValue *strs, MxcValue *vs, int ne) {
  MTable *table = mxc_alloc(sizeof(MTable));
  int default_capa = roundup(ne, 8);
  table->e = malloc(sizeof(struct mentry *) * default_capa);
  table->capa = default_capa;
  table->nentry = ne;
  SYSTEM(table) = &table_sys;

  for(int i = 0; i < ne; i++) {
    table->e[i] = new_entry(strs[i], vs[i]);
  }

  return mval_obj(table);
}

MxcValue new_table_capa(int capa) {
  MTable *table = mxc_alloc(sizeof(MTable));
  SYSTEM(table) = &table_sys;
  table->e = malloc(sizeof(struct mentry *) * capa);
  table->capa = capa;
  table->nentry = 0;

  return mval_obj(table);
}

void mtable_add(MTable *t, MxcValue key, MxcValue val) {
  /* TODO: length check */
  t->e[t->nentry++] = new_entry(key, val);
}

static void table_dealloc(MxcObject *a) {
  MTable *t = (MTable *)a;
  Mxc_free(t);
}

static void table_gc_mark(MxcObject *a) {
  if(a->marked) return;
  a->marked = 1;
}

static void table_gc_guard(MxcObject *a) {
  a->gc_guard = 1;
}

static void table_gc_unguard(MxcObject *a) {
  a->gc_guard = 0;
}

static MxcValue table_tostring(MxcObject *a) {
  MxcValue res;
  MTable *t = (MTable *)a;
  GC_GUARD(t);

  if(t->nentry == 0) {
    res = new_string_static("{}", 2);
    GC_UNGUARD(t);
    return res;
  }

  res = new_string_static("{", 1);
  mgc_guard(res);

  for(int i = 0; i < t->nentry; i++) {
    if(i > 0)
      str_cstr_append(ostr(res), ", ", 2);
    MxcValue key_s = mval2str(t->e[i]->key);
    MxcValue val_s = mval2str(t->e[i]->val);
    str_append(ostr(res), ostr(key_s));
    str_cstr_append(ostr(res), "=>", 2);
    str_append(ostr(res), ostr(val_s));
  }

  str_cstr_append(ostr(res), "}", 1);

  GC_UNGUARD(t);
  mgc_unguard(res);

  return res;
}

struct mobj_system table_sys = {
  "table",
  table_tostring,
  table_dealloc,
  0,  // TODO
  table_gc_mark,
  table_gc_guard,
  table_gc_unguard,
  0,
  0,
  0,
  0,
  0,
};
