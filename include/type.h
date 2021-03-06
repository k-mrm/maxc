#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include <stdbool.h>

#include "struct.h"
#include "util.h"
#include "operator.h"

#define MXCTYPE_HEADER  Type parent;

enum ttype {
  CTYPE_NONE,
  CTYPE_INT,
  CTYPE_UINT,
  CTYPE_INT64,
  CTYPE_UINT64,
  CTYPE_FLOAT,
  CTYPE_BOOL,
  CTYPE_CHAR,
  CTYPE_STRING,
  CTYPE_LIST,
  CTYPE_TABLE,
  CTYPE_TUPLE,
  CTYPE_RANGE,
  CTYPE_FUNCTION,
  CTYPE_GENERATOR,
  CTYPE_ITERATOR,
  CTYPE_UNINFERRED,
  CTYPE_ANY_VARARG,
  CTYPE_ANY,
  CTYPE_UNSOLVED,
  CTYPE_STRUCT,
  CTYPE_OPTIONAL,
  CTYPE_ERROR,
  CTYPE_FILE,
  /* userdef */
  CTYPE_USERDEF,
  /* type variable */
  CTYPE_VARIABLE,
};

enum typeimpl {
  T_SHOWABLE = 1 << 0,
  T_ITERABLE = 1 << 1,
  T_SUBSCRIPTABLE = 1 << 2,
};

typedef struct Type Type;

typedef char *(*type2str_t)(Type *);

struct Type {
  enum ttype type;
  enum typeimpl impl;
  type2str_t tostring;
  bool optional;

  union {
    /* range */
    struct {
      Type *ptr;
    };
    /* tuple */
    struct {
      Vector *tuple;
    };
    /* list, table */
    struct {
      Type *key;
      Type *val;
    };
    /* function */
    struct {
      Type *fnret;
      Vector *fnarg;
    };
    /* struct */
    struct {
      MxcStruct strct;
      char *name;
    };
    /* variable */
    struct {
      Type *real;
      char *vname;
    };
    /* userdef type */
    struct {
      char *uname;
    };
  };
};

Type *new_type(enum ttype);
Type *new_type_function(Vector *, Type *);
Type *new_type_generator(Vector *, Type *);
Type *new_type_iter(Type *);
Type *new_type_list(Type *);
Type *new_type_table(Type *, Type *);
Type *new_type_unsolved(char *);
Type *new_type_struct(MxcStruct);
Type *new_typevariable(char *);
Type *typevar(char *);
bool has_tvar(Type *);

Type *userdef_type(char *name, enum typeimpl impl);

char *vec_tyfmt(Vector *ty);
Type *typedup(Type *);

bool same_type(Type *, Type *);
bool is_struct(Type *);
bool unsolved(Type *);
Type *checktype(Type *, Type *);
bool type_is(Type *, enum ttype);
bool is_iterable(Type *);
bool is_subscriptable(Type *t);
Type *prune(Type *);

#define mxc_none (&TypeNone)
#define mxc_bool (&TypeBool)
#define mxc_char (&TypeChar)
#define mxc_int (&TypeInt)
#define mxc_float (&TypeFloat)
#define mxc_string (&TypeString)
#define mxc_file (&TypeFile)
#define mxc_any (&TypeAny)
#define mxc_any_vararg (&TypeAnyVararg)

#define typefmt(ty) (((Type *)ty)->tostring((Type *)ty))

extern Type TypeNone;
extern Type TypeBool;
extern Type TypeChar;
extern Type TypeInt;
extern Type TypeFloat;
extern Type TypeString;
extern Type TypeFile;
extern Type TypeAny;
extern Type TypeAnyVararg;

#endif
