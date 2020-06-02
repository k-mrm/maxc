#include <stdlib.h>
#include <string.h>

#include "scope.h"
#include "ast.h"
#include "error/error.h"
#include "builtins.h"

Scope *make_scope(Scope *s, int fblock) {
  Scope *n = malloc(sizeof(Scope));
  n->parent = s;
  n->vars = new_vector();
  n->userdef_type = new_vector();
  if(fblock == func_block) {
    n->fscope_vars = new_vector();
  }
  else {
    n->fscope_vars = s->fscope_vars;
  }
  n->fblock = fblock;
  if(!s || (s->fscope_gbl && fblock == local_scope))
    n->fscope_gbl = 1;
  return n;
}

Scope *scope_escape(Scope *s) {
  /*
  for(int i = 0; i < s->vars->len; i++) {
    NodeVariable *v = (NodeVariable *)s->vars->data[i]; 
    if(!v->used && !v->isbuiltin) {
      warn("unused variable: %s", v->name);
    }
  }
  */
  return s->parent;
}

int chk_var_conflict(Scope *s, NodeVariable *v) {
  Vector *vars = s->vars;

  for(int i = 0; i < vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)vars->data[i];

    if(strcmp(cur->name, v->name) == 0) {
      return 1;
    }
  }

  return 0;
} 

void varlist_show(Scope *s) {
  debug("varlist show: ");
  for(int i = 0; i < s->vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)s->vars->data[i];
    if(!cur) {
      printf("%s ", "null");
      continue;
    }
    do {
      printf("%s ", cur->name);
    } while((cur = cur->next));
  }
  puts("");
}

void scope_push_var(Scope *scope, NodeVariable *var) {
  vec_push(scope->vars, var);
  vec_push(scope->fscope_vars, var);
}

size_t var_set_number(Scope *s) {
  size_t id = 0;
  for(size_t i = 0; i < s->fscope_vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)s->fscope_vars->data[i];
    do {
      cur->vid = id++;
    } while((cur = cur->next));
  }

  return id;
}
