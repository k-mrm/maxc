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

static char *charty_tostring(Type *ty) { (void)ty; return "char"; } 

static char *floatty_tostring(Type *ty) { (void)ty; return "float"; } 

static char *stringty_tostring(Type *ty) { (void)ty; return "string"; }

static char *filety_tostring(Type *ty) { (void)ty; return "file"; }

static char *anyty_tostring(Type *ty) { (void)ty; return "any"; }

static char *any_varargty_tostring(Type *ty) { (void)ty; return "any_vararg"; }

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

  for(size_t i = 0; i < ty->fnarg->len; ++i) {
    Type *c = (Type *)ty->fnarg->data[i];
    sum_len += strlen(typefmt(c));
  }
  sum_len += strlen(typefmt(ty->fnret));

  sum_len += ty->fnarg->len + 2;
  /*
   *  2 is -1 + 3
   *  fnarg->len - 1 -> number of ','
   *  3 -> '(', ')', ':'
   */
  char *name = xmalloc(sum_len + 1);
  /*
   *  (int,int,int):int
   */
  strcpy(name, "(");
  for(size_t i = 0; i < ty->fnarg->len; ++i) {
    if(i > 0) {
      strcat(name, ",");
    }
    strcat(name, typefmt(ty->fnarg->data[i]));
  }
  strcat(name, "):");
  strcat(name, typefmt(ty->fnret));

  return name;
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

Type *new_type_iter(Vector *fnarg, Type *fnret) {
  Type *type = (Type *)xmalloc(sizeof(Type));
  type->type = CTYPE_ITERATOR;
  type->tostring = functy_tostring;
  type->fnarg = fnarg;
  type->fnret = fnret;
  type->impl = TIMPL_SHOW;
  type->optional = false;
  type->isprimitive = false;

  return type;
}

Type *new_type_list(Type *ty) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_LIST;
  type->tostring = listty_tostring;
  type->key = mxcty_int;
  type->val = ty;
  type->impl = TIMPL_SHOW | TIMPL_ITERABLE; 
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
  type->impl = TIMPL_SHOW;
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

bool type_is(Type *self, enum ttype ty) {
  return self && self->type == ty;
}

bool is_number(Type *t) {
  return t && (t->type == CTYPE_INT || 
      t->type == CTYPE_FLOAT);
}

bool is_unsolved(Type *t) {
  return t && t->type == CTYPE_UNSOLVED;
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

bool same_type(Type *t1, Type *t2) {
  return !!checktype(t1, t2);
}

Type *checktype(Type *ty1, Type *ty2) {
  if(!ty1 || !ty2) return NULL;

  ty1 = solvetype(ty1);
  ty2 = solvetype(ty2);

  if(type_is(ty1, CTYPE_LIST)) {
    if(!type_is(ty2, CTYPE_LIST))
      goto err;

    Type *a = ty1;
    Type *b = ty2;

    for(;;) {
      a = a->val;
      b = b->val;

      if(!a && !b)
        return ty1;
      if(!a || !b)
        goto err;
      if(!checktype(a, b))
        goto err;
    }
  }
  else if(ty1->type == CTYPE_STRUCT &&
      ty2->type == CTYPE_STRUCT) {
    if(strcmp(ty1->strct.name, ty2->strct.name) == 0)
      return ty1;
    else
      goto err;
  }
  else if(type_is(ty1, CTYPE_TUPLE)) {
    if(!type_is(ty2, CTYPE_TUPLE))
      goto err;
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
    if(ty1->fnret->type != ty2->fnret->type)
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


/* type */

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

Type TypeChar = {
  .type = CTYPE_CHAR,
  .impl = TIMPL_SHOW,
  .tostring = charty_tostring,
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
  .impl = TIMPL_SHOW,
  .tostring = stringty_tostring,
  .optional = false,
  .isprimitive = true,
  .key = mxcty_int,
  .val = mxcty_char,
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

