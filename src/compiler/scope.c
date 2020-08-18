#include <stdlib.h>
#include <string.h>

#include "scope.h"
#include "ast.h"
#include "error/error.h"

Scope *make_scope(Scope *s, enum scopetype ty) {
  Scope *n = malloc(sizeof(Scope));
  n->parent = s;
  n->vars = new_vector();
  n->userdef_type = new_vector();

  if(ty == FUNCSCOPE)
    n->fscope_vars = new_vector();
  else if(ty == BLOCKSCOPE)
    n->fscope_vars = s->fscope_vars;

  n->type = ty;

  if(!s || (s->fscope_gbl && ty == BLOCKSCOPE))
    n->fscope_gbl = true;
  else
    n->fscope_gbl = false;

  return n;
}

Scope *scope_escape(Scope *s) {
  Scope *parent = s->parent;
  /*
  for(int i = 0; i < s->vars->len; i++) {
    NodeVariable *v = (NodeVariable *)s->vars->data[i]; 
    if(!v->used && !v->isbuiltin) {
      warn("unused variable: %s", v->name);
    }
  }
  */
  del_vector(s->vars);
  del_vector(s->userdef_type);
  free(s);
  return parent;
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
  log_dbg("varlist show: ");
  for(int i = 0; i < s->vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)s->vars->data[i];
    if(!cur) {
      log_dbg("null ");
      continue;
    }
    do {
      log_dbg("%s ", cur->name);
    } while((cur = cur->next));
  }
  log_dbg("\n");
}

void scope_push_var(Scope *scope, NodeVariable *var) {
  vec_push(scope->vars, var);
  vec_push(scope->fscope_vars, var);
}

size_t var_assign_id(Scope *s) {
  size_t id = 0;
  for(size_t i = 0; i < s->fscope_vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)s->fscope_vars->data[i];
    do {
      cur->vid = id++;
    } while((cur = cur->next));
  }

  return id;
}
