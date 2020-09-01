#ifndef MAXC_OPERATOR_H
#define MAXC_OPERATOR_H

#include "util.h"
#include "object/mfunc.h"

struct NodeFunction;
struct Type;
typedef struct NodeFunction NodeFunction;
typedef struct Type Type;

enum MXC_OPERATOR {
  OPE_BINARY,
  OPE_UNARY,
  OPE_INVALID = -1,
};

enum BINOP {
  BIN_ADD,
  BIN_SUB,
  BIN_MUL,
  BIN_DIV,
  BIN_MOD,
  BIN_EQ,
  BIN_NEQ,
  BIN_LT,
  BIN_LTE,
  BIN_GT,
  BIN_GTE,
  BIN_LAND,
  BIN_LOR,
  BIN_LSHIFT,
  BIN_RSHIFT,
  BIN_BXOR,
  BIN_DOTDOT,
  BIN_QUESTION,
  BINOP_INVALID = -1,
};

enum UNAOP {
  UNA_INC,
  UNA_DEC,
  UNA_PLUS,
  UNA_MINUS,
  UNA_NOT,    /* ! */
  UNAOP_INVALID = -1,
};

Type *operator_type(enum MXC_OPERATOR, int, Type *, Type *);
char *operator_dump(enum MXC_OPERATOR, int);
enum BINOP op_char1(char c);
enum BINOP op_char2(char c1, char c2);

#endif
