#include <stdarg.h>
#include <stdlib.h>
#include "module.h"
#include "util.h"
#include "internal.h"
#include "object/funcobject.h"

static MCimpl *make_cimpl(NodeVariable *, MxcValue);
static void cbltin_add_obj(MxcModule *, NodeVariable *, MxcValue);

MxcModule *new_mxcmodule(char *name) {
  MxcModule *m = malloc(sizeof(MxcModule));
  m->name = name;
  m->cimpl = new_vector();
  return m;
}

void define_cfunc(MxcModule *mod,
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
  cbltin_add_obj(mod, var, func);
}

void define_cconst(MxcModule *mod,
    char *name,
    MxcValue val,
    Type *ty) {
  NodeVariable *var = node_variable_with_type(name, 0, ty);
  cbltin_add_obj(mod, var, val);
}

static MCimpl *make_cimpl(NodeVariable *v, MxcValue i) {
  MCimpl *cimpl = xmalloc(sizeof(MCimpl));
  cimpl->var = v;
  cimpl->impl = i;

  return cimpl;
}

static void cbltin_add_obj(MxcModule *mod, NodeVariable *v, MxcValue i) {
  vec_push(mod->cimpl, make_cimpl(v, i));
}
