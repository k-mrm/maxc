#include <stdarg.h>
#include <stdlib.h>
#include "module.h"
#include "util.h"
#include "internal.h"
#include "object/funcobject.h"

MxcModule *new_mxcmodule(char *name) {
  MxcModule *m = malloc(sizeof(MxcModule));
  m->name = name;
  m->cbltins = new_vector();
  return m;
}

void define_cmethod(Vector *self,
    char *name,
    cfunction impl,
    Type *ret, ...) {
  NodeVariable *var;
  Type *fntype;
  Vector *args = new_vector();
  va_list argva;
  va_start(argva, ret);

  for(Type *t = va_arg(argva, Type *); t; t = va_arg(argva, Type *)) {
    vec_push(args, t);
  }

  fntype = new_type_function(args, ret);
  var = node_variable_with_type(name, 0, fntype);

  MxcValue func = new_cfunc(impl);
  cbltin_add_obj(self, var, func);
}

void define_cconst(Vector *self,
    char *name,
    MxcValue val,
    Type *ty) {
  NodeVariable *var = node_variable_with_type(name, 0, ty);
  cbltin_add_obj(self, var, val);
}

MxcCBltin *new_cbltin(NodeVariable *v, MxcValue i) {
  MxcCBltin *cbltin = xmalloc(sizeof(MxcCBltin));
  cbltin->var = v;
  cbltin->impl = i;

  return cbltin;
}

void cbltin_add_obj(Vector *self, NodeVariable *v, MxcValue i) {
  MxcCBltin *blt = new_cbltin(v, i);
  vec_push(self, blt);
}
