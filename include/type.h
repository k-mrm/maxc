#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include <stdbool.h>

#include "struct.h"
#include "util.h"
#include "operator.h"

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
  CTYPE_ITERATOR,
  CTYPE_UNINFERRED,
  CTYPE_ANY_VARARG,
  CTYPE_ANY,
  CTYPE_UNSOLVED,
  CTYPE_STRUCT,
  CTYPE_OPTIONAL,
  CTYPE_ERROR,
  CTYPE_FILE,
  /* type variable */
  CTYPE_VARIABLE,
};

enum typeimpl {
  TIMPL_SHOW = 1 << 0,
  TIMPL_ITERABLE = 1 << 1,
};

typedef struct Type Type;

typedef char *(*type2str_t)(Type *);

struct Type {
  enum ttype type;
  enum typeimpl impl;
  type2str_t tostring;
  bool optional;
  bool isprimitive;

  union {
    /* range */
    struct {
      Type *ptr;
    };
    /* tuple */
    struct {
      Vector *tuple;
    };
    /* table */
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
  };
};

Type *new_type(enum ttype);
Type *new_type_function(Vector *, Type *);
Type *new_type_iter(Vector *, Type *);
Type *new_type_list(Type *);
Type *new_type_table(Type *, Type *);
Type *new_type_unsolved(char *);
Type *new_type_struct(MxcStruct);
Type *new_typevariable(char *);
Type *typevar(char *);

char *vec_tyfmt(Vector *ty);

bool same_type(Type *, Type *);
bool is_struct(Type *);
bool is_unsolved(Type *);
Type *checktype(Type *, Type *);
bool type_is(Type *, enum ttype);
bool is_iterable(Type *);

#define mxcty_none (&TypeNone)
#define mxcty_bool (&TypeBool)
#define mxcty_char (&TypeChar)
#define mxcty_int (&TypeInt)
#define mxcty_float (&TypeFloat)
#define mxcty_string (&TypeString)
#define mxcty_file (&TypeFile)
#define mxcty_any (&TypeAny)
#define mxcty_any_vararg (&TypeAnyVararg)

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
