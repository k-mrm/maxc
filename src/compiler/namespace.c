#include <string.h>

#include "internal.h"
#include "namespace.h"

Vector *namespace_table = NULL;

static Namespace *new_namespace(char *n, Varlist *l) {
  Namespace *ns = xmalloc(sizeof(Namespace));
  ns->name = n;
  ns->vars = l;

  return ns;
}

void Register_Namespace(char *n, Varlist *l) {
  if(!namespace_table) {
    namespace_table = New_Vector();
  }
  Namespace *ns = new_namespace(n, l);

  vec_push(namespace_table, ns);
}

Varlist *Search_Namespace(char *name) {
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
