#include "operator.h"
#include "error/error.h"
#include "type.h"
#include "object/mstr.h"

MxcOperator opdefs_integer[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_ADD,   mxcty_int, mxcty_int,   NULL,   "+"},
  {OPE_BINARY, BIN_SUB,   mxcty_int, mxcty_int,   NULL,   "-"},
  {OPE_BINARY, BIN_MUL,   mxcty_int, mxcty_int,   NULL,   "*"},
  {OPE_BINARY, BIN_DIV,   mxcty_int, mxcty_int,   NULL,   "/"},
  {OPE_BINARY, BIN_MOD,   mxcty_int, mxcty_int,   NULL,   "%"},
  {OPE_BINARY, BIN_EQ,    mxcty_int, mxcty_bool,  NULL,   "=="},
  {OPE_BINARY, BIN_NEQ,   mxcty_int, mxcty_bool,  NULL,   "!="},
  {OPE_BINARY, BIN_LT,    mxcty_int, mxcty_bool,  NULL,   "<"},
  {OPE_BINARY, BIN_LTE,   mxcty_int, mxcty_bool,  NULL,   "<="},
  {OPE_BINARY, BIN_GT,    mxcty_int, mxcty_bool,  NULL,   ">"},
  {OPE_BINARY, BIN_GTE,   mxcty_int, mxcty_bool,  NULL,   ">="},
  {OPE_BINARY, BIN_LAND,  mxcty_int, mxcty_bool,  NULL,   "and"},
  {OPE_BINARY, BIN_LOR,   mxcty_int, mxcty_bool,  NULL,   "or"},
  {OPE_BINARY, BIN_LSHIFT,mxcty_int, mxcty_int,   NULL,   "<<"},
  {OPE_BINARY, BIN_RSHIFT,mxcty_int, mxcty_int,   NULL,   ">>"},
  {OPE_BINARY, BIN_BXOR,  mxcty_int, mxcty_int,   NULL,   "xor"},
  {OPE_UNARY,  UNA_INC,   NULL,      mxcty_int,   NULL,   "++"},
  {OPE_UNARY,  UNA_DEC,   NULL,      mxcty_int,   NULL,   "--"},
  {OPE_UNARY,  UNA_MINUS, NULL,      mxcty_int,   NULL,   "-"},
  {-1, -1, NULL, NULL, NULL, NULL}
};

MxcOperator opdefs_boolean[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_EQ,    mxcty_bool, mxcty_bool, NULL,   "=="},
  {OPE_BINARY, BIN_NEQ,   mxcty_bool, mxcty_bool, NULL,   "!="},
  {OPE_BINARY, BIN_LAND,  mxcty_bool, mxcty_bool, NULL,   "and"},
  {OPE_BINARY, BIN_LOR,   mxcty_bool, mxcty_bool, NULL,   "or"},
  {OPE_UNARY,  UNA_NOT,   NULL,       mxcty_bool, NULL,   "!"},
  {-1, -1, NULL, NULL, NULL, NULL},
};

MxcOperator opdefs_float[] = {
  /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
  {OPE_BINARY, BIN_ADD,   mxcty_float, mxcty_float, NULL, "+"},
  {OPE_BINARY, BIN_SUB,   mxcty_float, mxcty_float, NULL, "-"},
  {OPE_BINARY, BIN_MUL,   mxcty_float, mxcty_float, NULL, "*"},
  {OPE_BINARY, BIN_DIV,   mxcty_float, mxcty_float, NULL, "/"},
  {OPE_BINARY, BIN_EQ,    mxcty_float, mxcty_bool,  NULL, "=="},
  {OPE_BINARY, BIN_NEQ,   mxcty_float, mxcty_bool,  NULL, "!="},
  {OPE_BINARY, BIN_LT,    mxcty_float, mxcty_bool,  NULL, "<"},
  {OPE_BINARY, BIN_LTE,   mxcty_float, mxcty_bool,  NULL, "<="},
  {OPE_BINARY, BIN_GT,    mxcty_float, mxcty_bool,  NULL, ">"},
  {OPE_BINARY, BIN_GTE,   mxcty_float, mxcty_bool,  NULL, ">="},
  {OPE_UNARY,  UNA_MINUS, NULL,        mxcty_float, NULL, "-"},
  {-1, -1, NULL, NULL, NULL, NULL},
};

MxcOperator opdefs_string[] = {
  /* kind */  /* ope */   /* ope2 */    /* ret */    /* fn */ /* opname */
  {OPE_BINARY, BIN_ADD,   mxcty_string, mxcty_string, NULL,   "+"},
  {OPE_BINARY, BIN_EQ,    mxcty_string, mxcty_bool,   NULL, "=="},
  {-1, -1, NULL, NULL, NULL, NULL},
};

MxcOperator *chk_operator_type(MxcOperator *self,
    enum MXC_OPERATOR kind,
    int op,
    Type *operand2) {
  if(!self) return NULL;

  MxcOperator *cur;
  for(int i = 0; self[i].kind != -1; ++i) {
    cur = &self[i];
    if(cur->kind != kind || cur->op != op)
      continue;
    if(operand2 && !same_type(cur->operand2, operand2))
      continue;

    return cur;
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
      case BIN_ADD:   return "+";
      case BIN_SUB:   return "-";
      case BIN_MUL:   return "*";
      case BIN_DIV:   return "/";
      case BIN_MOD:   return "%";
      case BIN_EQ:    return "==";
      case BIN_NEQ:   return "!=";
      case BIN_LT:    return "<";
      case BIN_LTE:   return "<=";
      case BIN_GT:    return ">";
      case BIN_GTE:   return ">=";
      case BIN_LAND:  return "&&";
      case BIN_LOR:   return "||";
      case BIN_LSHIFT:return "<<";
      case BIN_RSHIFT:return ">>";
      default:        return "error!";
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
  /* unreachable */
}
