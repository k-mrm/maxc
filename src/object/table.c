#include <stdint.h>
#include "object/mtable.h"

#define roundup(n, m) (n + (n % m? m - n % m : 0))

/* FNV-1 algorithm */
static uint32_t hash_32(char *key, size_t len) {
  uint32_t hash = 2166136261;
  
  for(size_t i = 0; i < len; i++) {
    hash = (hash ^ key[i]) * 16777619;
  }

  return hash;
}

static struct mentry *new_entry(MxcString *k, MxcValue v) {
  struct mentry *e = malloc(sizeof(mentry));
  e->key = hash_32(k->str, ITERABLE(k)->length);
  e->val = v;
  return e;
}

static struct mentry *search_val(MTable *t, uint32_t hash) {
  for(int i = 0; i < t->len; i++) {
    if(t->e[i]->key == hash)
      return t->e[i]->val;
  }

  return t->def;
}

MTable *new_table(MxcString **strs, MxcValue *vs, int len) {
  MTable *table = mxc_alloc(sizeof(MTable));
  int default_capa = roundup(len, 8);
  table->e = malloc(sizeof(struct mentry *) * default_capa);
  table->capa = default_capa;
  table->len = len;

  for(int i = 0; i < len; i++) {
    table->e[i] = new_entry(strs[i], vs[i]);
  }

  return table;
}
