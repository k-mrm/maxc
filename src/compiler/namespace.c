#include <string.h>

#include "internal.h"
#include "namespace.h"

Vector *namespace_table = NULL;

static Namespace *new_namespace(char *n, Vector *l) {
  Namespace *ns = xmalloc(sizeof(Namespace));
  ns->name = n;
  ns->vars = l;

  return ns;
}

void reg_namespace(char *n, Vector *l) {
  if(!namespace_table) {
    namespace_table = new_vector();
  }
  Namespace *ns = new_namespace(n, l);

  vec_push(namespace_table, ns);
}

Vector *search_namespace(char *name) {
  if(!namespace_table) {
    return NULL;
  }

  for(size_t i = 0; i < namespace_table->len; ++i) {
    Namespace *cur = (Namespace *)namespace_table->data[i];
    if(strcmp(cur->name, name) == 0) {
      return cur->vars;
    }
  }

  return NULL;
}
