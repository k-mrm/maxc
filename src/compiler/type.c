#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "error/error.h"
#include "maxc.h"

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
  char *t = ty->ptr->tostring(ty->ptr);
  char *name = xmalloc(strlen(t) + 3);
  sprintf(name, "[%s]", t);

  return name;
}

static char *tablety_tostring(Type *ty) {
  char *k = ty->key->tostring(ty->key);
  char *v = ty->val->tostring(ty->val);

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
    sum_len += strlen(c->tostring(c));
  }
  sum_len += strlen(ty->fnret->tostring(ty->fnret));

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
    strcat(name, ((Type *)ty->fnarg->data[i])->tostring((Type *)ty->fnarg->data[i]));
  }
  strcat(name, "):");
  strcat(name, ty->fnret->tostring(ty->fnret));

  return name;
}

static char *uninferty_tostring(Type *ty) {
  INTERN_UNUSE(ty);
  return "uninferred";
}

Type *new_type(enum ttype ty) {
  Type *type = (Type *)xmalloc(sizeof(Type));
  type->type = ty;

  if(ty == CTYPE_TUPLE) {
    type->tuple = new_vector();
    // type->tyname = "tuple";
    type->impl = 0;
  }
  else if(ty == CTYPE_ERROR) {
    type->err_msg = "";
    // type->tyname = "error";
    type->impl = TIMPL_SHOW;
  }
  else if(ty == CTYPE_UNINFERRED) {
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

Type *new_type_ptr(Type *ty) {
  Type *type = xmalloc(sizeof(Type));
  type->type = CTYPE_LIST;
  type->tostring = listty_tostring;
  type->ptr = ty;
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

Type *new_type_variable(char *name) {
  Type *type = xmalloc(sizeof(Type));

  static int id = 0;

  type->type = CTYPE_VARIABLE;
  type->id = id++;
  type->instance = NULL;
  type->optional = false;
  type->type_name = name;

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

Type *instantiate(Type *ty) {
  if(is_variable(ty)) {
    if(ty->instance != NULL) {
      ty->instance = instantiate(ty->instance);
      return ty->instance;
    }
  }

  return ty;
}

bool same_type(Type *t1, Type *t2) {
  if(!t1 || !t2) return false;

  t1 = instantiate(t1);
  t2 = instantiate(t2);

  if(t1->isprimitive) {
    return t1->type == t2->type;
  }
  else if(t1->type == CTYPE_STRUCT &&
      t2->type == CTYPE_STRUCT) {
    if(strncmp(t1->strct.name, t2->strct.name, strlen(t1->strct.name)) == 0) {
      return true;
    }
    else {
      return false;
    }
  }

  return false;
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
  {
    { .ptr = mxcty_char }
  },
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

