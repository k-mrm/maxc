#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "error/error.h"
#include "maxc.h"
#include "sema.h"

/* type tostring */

static char *nonety_tostring(Type *ty) { (void)ty; return "none"; }

static char *boolty_tostring(Type *ty) { (void)ty; return "bool"; } 

static char *intty_tostring(Type *ty) { (void)ty; return "int"; } 

static char *floatty_tostring(Type *ty) { (void)ty; return "float"; } 

static char *stringty_tostring(Type *ty) { (void)ty; return "string"; }

static char *filety_tostring(Type *ty) { (void)ty; return "file"; }

static char *anyty_tostring(Type *ty) { (void)ty; return "any"; }

static char *any_varargty_tostring(Type *ty) { (void)ty; return "any_vararg"; }

static char *tyvar_tostring(Type *ty) {
  if(ty->real) return typefmt(ty->real);
  else return ty->vname;
}

static char *structty_tostring(Type *ty) {
  char *pre = "object ";
  char *a = xmalloc(sizeof(char) * (strlen(pre) + strlen(ty->name) + 1));
  sprintf(a, "%s%s", pre, ty->name);
  return a;
}

static char *unsolvety_tostring(Type *ty) { 
  char *pre = "unsolved ";
  char *a = xmalloc(sizeof(char) * (strlen(pre) + strlen(ty->name) + 1));
  sprintf(a, "%s%s", pre, ty->name);
  return a;
}

static char *iterty_tostring(Type *ty) {
  char *t = typefmt(ty->ptr);
  char *name = xmalloc(strlen(t) + 7);
  sprintf(name, "iter(%s)", t);

  return name;
}

static char *listty_tostring(Type *ty) {
  char *t = typefmt(ty->val);
  char *name = xmalloc(strlen(t) + 3);
  sprintf(name, "[%s]", t);

  return name;
}

static char *tablety_tostring(Type *ty) {
  char *k = typefmt(ty->key);
  char *v = typefmt(ty->val);

  char *name = malloc(strlen(k) + strlen(v) + 5);
  sprintf(name, "[%s=>%s]", k, v);

  return name;
}

static char *functy_tostring(Type *ty) {
  size_t sum_len = 0;

  if(!ty->fnarg || !ty->fnret)
    return "";

  char *fargstr = vec_tyfmt(ty->fnarg);
  char *fretstr = typefmt(ty->fnret);
  sum_len += strlen(fargstr);
  sum_len += strlen(fretstr);
  sum_len += 3;
  /*
   *  3 -> '(', ')', ':'
   */
  char *buf = calloc(1, sum_len + 1);
  /*
   *  (int,int,int):int
   */
  strcpy(buf, "(");
  strcat(buf, fargstr);
  strcat(buf, "):");
  strcat(buf, fretstr);

  return buf;
}

char *vec_tyfmt(Vector *ty) {
  char *tmp[ty->len];
  unsigned int slen = 0;

  for(int i = 0; i < ty->len; i++) {
    tmp[i] = typefmt((Type *)ty->data[i]);
    slen += strlen(tmp[i]);
  }
  slen += ty->len;

  if(slen == 0)
    return "";

  char *buf = calloc(1, slen);

  for(int i = 0; i < ty->len; i++) {
    if(i > 0) {
      strcat(buf, ",");
    }
    strcat(buf, tmp[i]);
  }

  return buf;
}

static char *uninferty_tostring(Type *ty) {
  INTERN_UNUSE(ty);
  return "uninferred";
}

Type *new_type(enum ttype ty) {
  Type *type = (Type *)xmalloc(sizeof(Type));
  type->type = ty;

  if(ty == CTYPE_UNINFERRED) {
    type->tostring = uninferty_tostring;
    type->impl = 0;
  }

  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_function(Vector *fnarg, Type *fnret) {
  Type *type = (Type *)xmalloc(sizeof(Type));
  type->type = CTYPE_FUNCTION;
  type->tostring = functy_tostring;
  type->fnarg = fnarg;
  type->fnret = fnret;
  type->impl = TIMPL_SHOW;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_generator(Vector *fnarg, Type *fnret) {
  Type *type = (Type *)xmalloc(sizeof(Type));
  type->type = CTYPE_GENERATOR;
  type->tostring = functy_tostring;
  type->fnarg = fnarg;
  type->fnret = new_type_iter(fnret);
  type->impl = TIMPL_SHOW;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_iter(Type *ty) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_ITERATOR;
  type->tostring = iterty_tostring;
  type->ptr = ty;
  type->impl = TIMPL_SHOW | TIMPL_ITERABLE; 
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_list(Type *ty) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_LIST;
  type->tostring = listty_tostring;
  type->key = mxc_int;
  type->val = ty;
  type->impl = TIMPL_SHOW | TIMPL_ITERABLE | TIMPL_SUBSCRIPTABLE; 
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_table(Type *k, Type *v) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_TABLE;
  type->tostring = tablety_tostring;
  type->key = k;
  type->val = v;
  type->impl = TIMPL_SHOW | TIMPL_SUBSCRIPTABLE;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_unsolved(char *str) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_UNSOLVED;
  type->impl = 0;
  type->name = str;
  type->tostring = unsolvety_tostring;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_struct(MxcStruct strct) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_STRUCT;
  type->name = strct.name;
  type->tostring = structty_tostring;
  type->impl = 0;
  type->strct = strct;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_typevariable(char *vname) {
  Type *type = malloc(sizeof(Type));
  type->type = CTYPE_VARIABLE;
  type->tostring = tyvar_tostring;
  type->vname = vname;
  type->impl = 0;
  type->optional = false;
  type->isprimitive = false;
  type->real = NULL;

  return type;
}

Type *typevar(char *name) {
  return new_typevariable(name);
}

Type *prune(Type *v) {
  if(type_is(v, CTYPE_VARIABLE) && v->real) {
    return prune(v->real);
  }
  return v;
}

bool has_tvar(Type *t) {
  switch(t->type) {
    case CTYPE_VARIABLE: t->real = NULL; return true;
    case CTYPE_LIST: return has_tvar(t->val);
    case CTYPE_TABLE:
      return has_tvar(t->key) || has_tvar(t->val);
    case CTYPE_FUNCTION: /* TODO */ return false;
    default: return false;
  }
}

Type *typedup(Type *t) {
  Type *n = malloc(sizeof(Type));
  *n = *t;
  switch(t->type) {
    case CTYPE_LIST:
    case CTYPE_TABLE:
      n->key = typedup(t->key);
      n->val = typedup(t->val);
      break;
    case CTYPE_FUNCTION: {
      n->fnret = typedup(t->fnret);
      n->fnarg = new_vector();
      for(int i = 0; i < t->fnarg->len; i++) {
        vec_push(n->fnarg, typedup((Type *)t->fnarg->data[i]));
      }
      break;
    }
    case CTYPE_VARIABLE:
      n->real = typedup(t->real);
      break;
    default:
      break;
  }
  return n;
}

bool type_is(Type *self, enum ttype ty) {
  return self && self->type == ty;
}

bool is_number(Type *t) {
  return t && (t->type == CTYPE_INT || t->type == CTYPE_FLOAT);
}

bool unsolved(Type *t) {
  if(!t) {
    return false;
  }

  switch(t->type) {
    case CTYPE_UNSOLVED: return true;
    case CTYPE_LIST:
      return unsolved(t->key) || unsolved(t->val);
    case CTYPE_FUNCTION: {
      bool ret = unsolved(t->fnret);
      for(int i = 0; i < t->fnarg->len; i++) {
        ret = ret || unsolved((Type *)t->fnarg->data[i]);
      }
      return ret;
    }
    default: return false;
  }
}

bool is_variable(Type *t) {
  return t && (t->type == CTYPE_VARIABLE);
}

bool is_struct(Type *t) {
  return t && (t->type == CTYPE_STRUCT);
}

bool is_iterable(Type *t) {
  return t && (t->impl & TIMPL_ITERABLE); 
}

bool is_subscriptable(Type *t) {
  return t && (t->impl & TIMPL_SUBSCRIPTABLE); 
}

bool same_type(Type *t1, Type *t2) {
  return !!checktype(t1, t2);
}

Type *checktype(Type *ty1, Type *ty2) {
  if(!ty1 || !ty2) return NULL;

  ty1 = solvetype(ty1);
  ty2 = solvetype(ty2);

  if(type_is(ty1, CTYPE_ANY) || type_is(ty2, CTYPE_ANY)) {
    return mxc_any;
  }
  else if(type_is(ty1, CTYPE_VARIABLE)) {
    if(ty1->real) {
      Type *c = checktype(ty1->real, ty2);
      return c;
    }
    else {
      ty1->real = ty2;
      return ty2;
    }
  }
  else if(type_is(ty2, CTYPE_VARIABLE)) {
    if(ty2->real) {
      Type *c = checktype(ty2->real, ty1);
      return c;
    }
    else {
      ty2->real = ty1;
      return ty1;
    }
  }
  else if(type_is(ty1, CTYPE_LIST) && type_is(ty2, CTYPE_LIST)) {
    Type *a = ty1;
    Type *b = ty2;

    for(;;) {
      if(type_is(a, CTYPE_VARIABLE) || type_is(b, CTYPE_VARIABLE)) {
        return ty1;
      }

      a = a->val;
      b = b->val;

      if(!a && !b)
        return ty1;
      if(!a || !b)
        goto err;
      Type *chk = checktype(a, b);
      if(!chk)
        goto err;
      if(type_is(chk, CTYPE_ANY))
        return mxc_any;
    }
  }
  else if(type_is(ty1, CTYPE_STRUCT) && type_is(ty2, CTYPE_STRUCT)) {
    if(!strcmp(ty1->strct.name, ty2->strct.name))
      return ty1;
    else
      goto err;
  }
  else if(type_is(ty1, CTYPE_TUPLE) && type_is(ty2, CTYPE_TUPLE)) {
    if(ty1->tuple->len != ty2->tuple->len)
      goto err;
    int s = ty1->tuple->len;
    int cnt = 0;

    for(;;) {
      checktype(ty1->tuple->data[cnt], ty2->tuple->data[cnt]);
      ++cnt;
      if(cnt == s)
        return ty1;
    }
  }
  else if(type_is(ty1, CTYPE_FUNCTION)) {
    if(!type_is(ty2, CTYPE_FUNCTION))
      goto err;
    if(ty1->fnarg->len != ty2->fnarg->len)
      goto err;
    if(!checktype(ty1->fnret, ty2->fnret))
      goto err;

    int i = ty1->fnarg->len;
    int cnt = 0;

    if(i == 0)
      return ty1;

    for(;;) {
      if(!checktype(ty1->fnarg->data[cnt], ty2->fnarg->data[cnt])) {
        if(!ty1->fnarg->data[cnt] || !ty2->fnarg->data[cnt])
          return NULL;
        Type *err1 = (Type *)ty1->fnarg->data[cnt];
        Type *err2 = (Type *)ty2->fnarg->data[cnt];

        error("type error `%s`, `%s`", typefmt(err1), typefmt(err2));
      }
      ++cnt;
      if(cnt == i)
        return ty1;
    }
  }
  else if(ty1->type == ty2->type) {   // primitive type
    return ty1;
  }

err:
  return NULL;
}

/* primitive type definition */

Type TypeNone = {
  .type = CTYPE_NONE,
  .impl = TIMPL_SHOW,
  .tostring = nonety_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

Type TypeBool = {
  .type = CTYPE_BOOL,
  .impl = TIMPL_SHOW,
  .tostring = boolty_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

Type TypeInt = {
  .type = CTYPE_INT,
  .impl = TIMPL_SHOW,
  .tostring = intty_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
};

Type TypeFloat = {
  .type = CTYPE_FLOAT,
  .impl = TIMPL_SHOW,
  .tostring = floatty_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

Type TypeString = {
  .type = CTYPE_STRING,
  .impl = TIMPL_SHOW | TIMPL_ITERABLE | TIMPL_SUBSCRIPTABLE,
  .tostring = stringty_tostring,
  .optional = false,
  .isprimitive = true,
  .key = mxc_int,
  .val = mxc_string,
}; 

Type TypeFile = {
  .type = CTYPE_FILE,
  .impl = TIMPL_SHOW,
  .tostring = filety_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

Type TypeAny = {
  .type = CTYPE_ANY,
  .impl = 0,
  .tostring = anyty_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

Type TypeAnyVararg = {
  .type = CTYPE_ANY_VARARG,
  .impl = 0,
  .tostring = any_varargty_tostring,
  .optional = false,
  .isprimitive = true,
  {{0}},
}; 

