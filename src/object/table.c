#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "object/mtable.h"
#include "mem.h"

#define roundup(n, m) (n + (n % m? m - n % m : 0))

static int primes[] = {
  3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093
};

static int nslot_from(int c) {
  int nprimes = sizeof(primes) / sizeof(int);
  for(int i = 0; i < nprimes; i++) {
    if(c < primes[i])
      return primes[i];
  }

  return 0;
}

/* FNV-1 algorithm */
static uint32_t hash32(char *key, size_t len) {
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

MxcValue new_table_capa(int capa) {
  MTable *table = (MTable *)mxc_alloc(sizeof(MTable));
  SYSTEM(table) = &table_sys;
  int nslot = nslot_from(capa);
  table->e = malloc(sizeof(struct mentry *) * nslot);
  memset(table->e, 0, sizeof(*table->e));
  table->nslot = nslot;
  table->nentry = 0;

  return mval_obj(table);
}

void mtable_add(MTable *t, MxcValue key, MxcValue val) {
  t->nentry++;

  if(t->nentry > t->nslot) {
    // TODO
  }

  int i = hash32(ostr(key)->str, ITERABLE(V2O(key))->length) % t->nslot;
  t->e[i] = new_entry(key, val);
  printf("key! %d\n", i);
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

  int c = 0;
  for(int i = 0; i < t->nslot; i++) {
    if(!t->e[i])
      continue;

    if(c++ > 0)
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
