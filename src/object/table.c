#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "object/mtable.h"
#include "mem.h"

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

static void rehash(MTable *t) {
  ;
}

static struct mentry *new_entry(MxcValue k, MxcValue v) {
  struct mentry *e = malloc(sizeof(struct mentry));
  e->key = k;
  e->val = v;
  e->next = NULL;
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

static void echain_add(struct mentry *e, struct mentry *new) {
  struct mentry *end = e;
  while(end->next) {
    end = end->next;
  }
  end->next = new;
}

void mtable_add(MTable *t, MxcValue key, MxcValue val) {
  t->nentry++;

  if(t->nentry > t->nslot) {
    t->nslot = nslot_from(t->nentry);
    rehash(t);
  }

  uint32_t i = hash32(ostr(key)->str, ITERABLE(V2O(key))->length) % t->nslot;
  struct mentry *new = new_entry(key, val);
  if(t->e[i])
    echain_add(t->e[i], new);
  else
    t->e[i] = new;
}

static MxcValue mtable_at(MTable *t, MxcValue key) {
  uint32_t i = hash32(ostr(key)->str, ITERABLE(V2O(key))->length) % t->nslot;
  struct mentry *e = t->e[i];
  return e->val;  // TODO: collision
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
    for(struct mentry *e = t->e[i]; e; e = e->next) {
      if(c++ > 0)
        str_cstr_append(ostr(res), ", ", 2);
      MxcValue key_s = mval2str(e->key);
      MxcValue val_s = mval2str(e->val);
      str_append(ostr(res), ostr(key_s));
      str_cstr_append(ostr(res), "=>", 2);
      str_append(ostr(res), ostr(val_s));
    }
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
