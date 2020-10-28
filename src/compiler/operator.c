#include "operator.h"
#include "sema.h"
#include "error/error.h"
#include "type.h"
#include "object/mstr.h"

struct moperator {
  enum MXC_OPERATOR kind;
  int op;
  Type *operand2;
  Type *ret;
  void *func;
  char *opname;
};

static struct moperator opdefs_integer[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_ADD,   mxc_int, mxc_int,   NULL,   "+"},
  {OPE_BINARY, BIN_SUB,   mxc_int, mxc_int,   NULL,   "-"},
  {OPE_BINARY, BIN_MUL,   mxc_int, mxc_int,   NULL,   "*"},
  {OPE_BINARY, BIN_DIV,   mxc_int, mxc_int,   NULL,   "/"},
  {OPE_BINARY, BIN_MOD,   mxc_int, mxc_int,   NULL,   "%"},
  {OPE_BINARY, BIN_EQ,    mxc_int, mxc_bool,  NULL,   "=="},
  {OPE_BINARY, BIN_NEQ,   mxc_int, mxc_bool,  NULL,   "!="},
  {OPE_BINARY, BIN_LT,    mxc_int, mxc_bool,  NULL,   "<"},
  {OPE_BINARY, BIN_LTE,   mxc_int, mxc_bool,  NULL,   "<="},
  {OPE_BINARY, BIN_GT,    mxc_int, mxc_bool,  NULL,   ">"},
  {OPE_BINARY, BIN_GTE,   mxc_int, mxc_bool,  NULL,   ">="},
  {OPE_BINARY, BIN_LAND,  mxc_int, mxc_bool,  NULL,   "and"},
  {OPE_BINARY, BIN_LOR,   mxc_int, mxc_bool,  NULL,   "or"},
  {OPE_BINARY, BIN_LSHIFT,mxc_int, mxc_int,   NULL,   "<<"},
  {OPE_BINARY, BIN_RSHIFT,mxc_int, mxc_int,   NULL,   ">>"},
  {OPE_BINARY, BIN_BXOR,  mxc_int, mxc_int,   NULL,   "xor"},
  {OPE_UNARY,  UNA_INC,   NULL,      mxc_int,   NULL,   "++"},
  {OPE_UNARY,  UNA_DEC,   NULL,      mxc_int,   NULL,   "--"},
  {OPE_UNARY,  UNA_MINUS, NULL,      mxc_int,   NULL,   "-"},
  {-1, -1, NULL, NULL, NULL, NULL}
};

static struct moperator opdefs_boolean[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_EQ,    mxc_bool, mxc_bool, NULL,   "=="},
  {OPE_BINARY, BIN_NEQ,   mxc_bool, mxc_bool, NULL,   "!="},
  {OPE_BINARY, BIN_LAND,  mxc_bool, mxc_bool, NULL,   "and"},
  {OPE_BINARY, BIN_LOR,   mxc_bool, mxc_bool, NULL,   "or"},
  {OPE_UNARY,  UNA_NOT,   NULL,       mxc_bool, NULL,   "!"},
  {-1, -1, NULL, NULL, NULL, NULL},
};

static struct moperator opdefs_float[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_ADD,   mxc_float, mxc_float, NULL, "+"},
  {OPE_BINARY, BIN_SUB,   mxc_float, mxc_float, NULL, "-"},
  {OPE_BINARY, BIN_MUL,   mxc_float, mxc_float, NULL, "*"},
  {OPE_BINARY, BIN_DIV,   mxc_float, mxc_float, NULL, "/"},
  {OPE_BINARY, BIN_EQ,    mxc_float, mxc_bool,  NULL, "=="},
  {OPE_BINARY, BIN_NEQ,   mxc_float, mxc_bool,  NULL, "!="},
  {OPE_BINARY, BIN_LT,    mxc_float, mxc_bool,  NULL, "<"},
  {OPE_BINARY, BIN_LTE,   mxc_float, mxc_bool,  NULL, "<="},
  {OPE_BINARY, BIN_GT,    mxc_float, mxc_bool,  NULL, ">"},
  {OPE_BINARY, BIN_GTE,   mxc_float, mxc_bool,  NULL, ">="},
  {OPE_UNARY,  UNA_MINUS, NULL,        mxc_float, NULL, "-"},
  {-1, -1, NULL, NULL, NULL, NULL},
};

static struct moperator opdefs_string[] = {
  /* kind */  /* ope */   /* ope2 */    /* ret */    /* fn */ /* opname */
  {OPE_BINARY, BIN_ADD,   mxc_string, mxc_string, NULL,   "+"},
  {OPE_BINARY, BIN_EQ,    mxc_string, mxc_bool,   NULL, "=="},
  {-1, -1, NULL, NULL, NULL, NULL},
};

static struct moperator *operator_definition(Type *ope1) {
  switch(ope1->type) {
    case CTYPE_INT: return opdefs_integer;
    case CTYPE_BOOL: return opdefs_boolean;
    case CTYPE_FLOAT: return opdefs_float;
    case CTYPE_STRING: return opdefs_string;
    default: return NULL;
  }
}

Type *operator_type(enum MXC_OPERATOR kind,
    int op,
    Type *ope1,
    Type *ope2) {
  struct moperator *defs = operator_definition(ope1);
  if(!defs)
    return NULL;

  if(op == BIN_QUESTION)
    return checktype(ope1, ope2);

  struct moperator *cur;
  for(int i = 0; defs[i].kind != -1; ++i) {
    cur = &defs[i];

    if(cur->kind != kind || cur->op != op)
      continue;
    if(ope2 && !same_type(cur->operand2, ope2))
      continue;

    return cur->ret;
  }
  return NULL;
}

enum BINOP op_char1(char c) {
  switch(c) {
    case '+': return BIN_ADD;
    case '-': return BIN_SUB;
    case '*': return BIN_MUL;
    case '/': return BIN_DIV;
    case '%': return BIN_MOD;
    case '<': return BIN_LT;
    case '>': return BIN_GT;
    default : return -1;
  }
}

enum BINOP op_char2(char c1, char c2) {
  switch(c1) {
    case '=':
      switch(c2) {
        case '=':
          return BIN_EQ;
      }
      break;
    case '<':
      switch(c2) {
        case '=':
          return BIN_LTE;
        case '<':
          return BIN_LSHIFT;
      }
      break;
    case '>':
      switch(c2) {
        case '=':
          return BIN_GTE;
        case '>':
          return BIN_RSHIFT;
      }
      break;
    case '!':
      switch(c2) {
        case '=':
          return BIN_NEQ;
      }
      break;
    default:
      return -1;
  }
}


char *operator_dump(enum MXC_OPERATOR k, int n) {
  if(k == OPE_BINARY) {
    switch(n) {
      case BIN_ADD:       return "+";
      case BIN_SUB:       return "-";
      case BIN_MUL:       return "*";
      case BIN_DIV:       return "/";
      case BIN_MOD:       return "%";
      case BIN_EQ:        return "==";
      case BIN_NEQ:       return "!=";
      case BIN_LT:        return "<";
      case BIN_LTE:       return "<=";
      case BIN_GT:        return ">";
      case BIN_GTE:       return ">=";
      case BIN_LAND:      return "&&";
      case BIN_LOR:       return "||";
      case BIN_LSHIFT:    return "<<";
      case BIN_RSHIFT:    return ">>";
      case BIN_QUESTION:  return "?";
      default:            return "error!";
    }
  }
  else if(k == OPE_UNARY) {
    switch(n) {
      case UNA_INC:   return "++";
      case UNA_DEC:   return "--";
      case UNA_PLUS:  return "+";
      case UNA_MINUS: return "-";
      case UNA_NOT:   return "!";
      default:        return "error!";
    }
  }
}
