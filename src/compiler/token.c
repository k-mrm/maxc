#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "error/error.h"
#include "maxc.h"
#include "operator.h"

static enum tkind ident2kw(char *);

struct keywordmap {
  char *key;
  enum tkind kind;
} mxc_kwmap[] = {
  {"int", TKIND_TInt},       {"bool", TKIND_TBool},
  {"string", TKIND_TString}, {"float", TKIND_TFloat},
  {"none", TKIND_TNone},     {"or", TKIND_KOr},
  {"and", TKIND_KAnd},       {"object", TKIND_Object},
  {"return", TKIND_Return},  {"if", TKIND_If},
  {"else", TKIND_Else},      {"for", TKIND_For},
  {"while", TKIND_While},    {"typedef", TKIND_Typedef},
  {"let", TKIND_Let},        {"def", TKIND_Fn},
  {"true", TKIND_True},      {"false", TKIND_False},
  {"const", TKIND_Const},    {"use", TKIND_Use},
  {"Error", TKIND_TError},   {"failure", TKIND_FAILURE},
  {"break", TKIND_Break},    {"skip", TKIND_Skip},
  {"new", TKIND_New},        {"in", TKIND_In},
  {"null", TKIND_Null},      {"breakpoint", TKIND_BreakPoint},
  {"xor", TKIND_Xor},        {"assert", TKIND_Assert},
  {"File", TKIND_TFile},
};

enum tkind tk_char1(int c) {
  switch(c) {
    case '+': return TKIND_Plus;
    case '-': return TKIND_Minus;
    case '*': return TKIND_Asterisk;
    case '/': return TKIND_Div;
    case '%': return TKIND_Mod;
    case '(': return TKIND_Lparen;
    case ')': return TKIND_Rparen;
    case '{': return TKIND_Lbrace;
    case '}': return TKIND_Rbrace;
    case '[': return TKIND_Lboxbracket;
    case ']': return TKIND_Rboxbracket;
    case ',': return TKIND_Comma;
    case ':': return TKIND_Colon;
    case ';': return TKIND_Semicolon;
    case '.': return TKIND_Dot;
    case '<': return TKIND_Lt;
    case '>': return TKIND_Gt;
    case '=': return TKIND_Assign;
    case '!': return TKIND_Bang;
    case '?': return TKIND_Question;
    case '@': return TKIND_Atmark;
    default:
              error("internal error: %c", c);
              return -1;
  }
}

enum tkind tk_char2(int c1, int c2) {
  switch(c1) {
    case '=':
      switch(c2) {
        case '=':
          return TKIND_Eq;
        case '>':
          return TKIND_FatArrow;
      }
      break;
    case '<':
      switch(c2) {
        case '=':
          return TKIND_Lte;
        case '<':
          return TKIND_Lshift;
      }
      break;
    case '>':
      switch(c2) {
        case '=':
          return TKIND_Gte;
        case '>':
          return TKIND_Rshift;
      }
      break;
    case '!':
      switch(c2) {
        case '=':
          return TKIND_Neq;
      }
      break;
    case '+':
      switch(c2) {
        case '+':
          return TKIND_Inc;
        case '=':
          return TKIND_PlusAs;
      }
      break;
    case '-':
      switch(c2) {
        case '-':
          return TKIND_Dec;
        case '=':
          return TKIND_MinusAs;
        case '>':
          return TKIND_Arrow;
      }
      break;
    case '*':
      switch(c2) {
        case '=':
          return TKIND_AsteriskAs;
      }
      break;
    case '&':
      switch(c2) {
        case '&':
          return TKIND_LogAnd;
      }
      break;
    case '|':
      switch(c2) {
        case '|':
          return TKIND_LogOr;
      }
      break;
    case '/':
      switch(c2) {
        case '=':
          return TKIND_DivAs;
      }
      break;
    case '%':
      switch(c2) {
        case '=':
          return TKIND_ModAs;
      }
      break;
    default:
      error("internal error: %c%c", c1, c2);
      return -1;
  }
}

const char *tk2str(enum tkind tk) {
  switch(tk) {
    case TKIND_End: return "End";
    case TKIND_Num: return "Number";
    case TKIND_String: return "String";
    case TKIND_Char: return "Char";
    case TKIND_Identifer: return "Identifer";
    case TKIND_TInt: return "int";
    case TKIND_TUint: return "uint";
    case TKIND_TInt64: return "int64";
    case TKIND_TUint64: return "uint64";
    case TKIND_TBool: return "bool";
    case TKIND_TChar: return "char";
    case TKIND_TString: return "string";
    case TKIND_TFloat: return "float";
    case TKIND_TError: return "Error";
    case TKIND_TNone: return "none";
    case TKIND_KAnd: return "and";
    case TKIND_KOr: return "or";
    case TKIND_Struct: return "struct";
    case TKIND_Data: return "data";
    case TKIND_Object: return "object";
    case TKIND_If: return "if";
    case TKIND_Else: return "else";
    case TKIND_For: return "for";
    case TKIND_While: return "while";
    case TKIND_Return: return "return";
    case TKIND_Let: return "let";
    case TKIND_Use: return "use";
    case TKIND_Fn: return "def";
    case TKIND_True: return "true";
    case TKIND_False: return "false";
    case TKIND_Const: return "const";
    case TKIND_FAILURE: return "failure";
    case TKIND_Break: return "break";
    case TKIND_Skip: return "skip";
    case TKIND_New: return "new";
    case TKIND_In: return "in";
    case TKIND_Null: return "null";
    case TKIND_BreakPoint: return "breakpoint";
    case TKIND_Xor: return "xor";
    case TKIND_Assert: return "assert";
    case TKIND_Lparen: return "(";
    case TKIND_Rparen: return ")";
    case TKIND_Lbrace: return "{";
    case TKIND_Rbrace: return "}";
    case TKIND_Lboxbracket: return "[";
    case TKIND_Rboxbracket: return "]";
    case TKIND_Comma: return ",";
    case TKIND_Colon: return ":";
    case TKIND_Dot: return ".";
    case TKIND_DotDot: return "..";
    case TKIND_Semicolon: return ";";
    case TKIND_Arrow: return "->";
    case TKIND_FatArrow: return "=>";
    case TKIND_Inc: return "++";
    case TKIND_Dec: return "--";
    case TKIND_Plus: return "+";
    case TKIND_Minus: return "-";
    case TKIND_Asterisk: return "*";
    case TKIND_Div: return "/";
    case TKIND_Mod: return "%";
    case TKIND_Eq: return "==";
    case TKIND_Neq: return "!=";
    case TKIND_Lt: return "<";
    case TKIND_Lte: return "<=";
    case TKIND_Gt: return ">";
    case TKIND_Gte: return ">=";
    case TKIND_LogAnd: return "&&";
    case TKIND_LogOr: return "||";
    case TKIND_Lshift: return "<<";
    case TKIND_Rshift: return ">>";
    case TKIND_Assign: return "=";
    case TKIND_Bang: return "!";
    case TKIND_Question: return "?";
    default: return "error";
  }
}

static Token *New_Token(enum tkind kind,
    String *value,
    SrcPos s,
    SrcPos e) {
  Token *self = malloc(sizeof(Token));

  self->kind = kind;
  self->value = s_tochar(value);
  self->len = value->len;
  self->start = s;
  self->end = e;

  return self;
}

static Token *New_Token_Char(char c, SrcPos s, SrcPos e) {
  Token *self = malloc(sizeof(Token));
  self->kind = TKIND_Char;
  self->cont = c;
  self->start = s;
  self->end = e;

  return self;
}

static Token *New_Token_With_Bq(
    enum tkind cont,
    uint8_t len,
    SrcPos s,
    SrcPos e
    ) {
  Token *self = malloc(sizeof(Token));
  self->kind = TKIND_BQLIT;
  self->cont = cont;
  self->len = len;
  self->start = s;
  self->end = e;

  return self;
}

static Token *New_Token_With_Symbol(enum tkind kind,
    uint8_t len,
    SrcPos s,
    SrcPos e) {
  Token *self = malloc(sizeof(Token));

  self->kind = kind;
  self->value = "";
  self->len = len;
  self->start = s;
  self->end = e;

  return self;
}

static Token *New_Token_With_End(SrcPos s, SrcPos e) {
  Token *self = malloc(sizeof(Token));

  self->kind = TKIND_End;
  self->value = "";
  self->len = 1;
  self->start = s;
  self->end = e;

  return self;
}

void token_push_num(Vector *self, String *value, SrcPos s, SrcPos e) {
  Token *tk = New_Token(TKIND_Num, value, s, e);

  vec_push(self, tk);
}

void token_push_ident(Vector *self, String *value, SrcPos s, SrcPos e) {
  enum tkind kind = ident2kw(value->data);

  Token *tk = New_Token(kind, value, s, e);

  vec_push(self, tk);
}

void token_push_symbol(
    Vector *self,
    enum tkind kind,
    uint8_t len,
    SrcPos s,
    SrcPos e
    ) {
  Token *tk = New_Token_With_Symbol(kind, len, s, e);

  vec_push(self, tk);
}

void token_push_string(Vector *self, String *str, SrcPos s, SrcPos e) {
  Token *tk = New_Token(TKIND_String, str, s, e);

  vec_push(self, tk);
}

void token_push_char(Vector *self, char c, SrcPos s, SrcPos e) {
  vec_push(self, New_Token_Char(c, s, e));
}

void token_push_backquote_lit(Vector *self,
    String *str,
    SrcPos s,
    SrcPos e) {
  enum tkind a = 0;

  if(str->len == 1) {
    a = op_char1(str->data[0]);
  }
  else if(str->len == 2) {
    a = op_char2(str->data[0], str->data[1]);
  }

  Token *tk = New_Token_With_Bq(a, str->len, s, e);

  vec_push(self, tk);
}

void token_push_end(Vector *self, SrcPos s, SrcPos e) {
  Token *tk = New_Token_With_End(s, e);

  vec_push(self, tk);
}

static enum tkind ident2kw(char *k) {
  static int kwlen = sizeof(mxc_kwmap) / sizeof(mxc_kwmap[0]);

  for(int i = 0; i < kwlen; i++) {
    if(strcmp(k, mxc_kwmap[i].key) == 0)
      return mxc_kwmap[i].kind;
  }
  return TKIND_Identifer;
}

#ifdef MXC_DEBUG
void tokendump(Vector *token) {
  for(int i = 0; i < token->len; ++i) {
    printf("kind: %s\t\t", tk2str(((Token *)token->data[i])->kind));
    printf("value: %s\t\t", ((Token *)token->data[i])->value);
    printf("len: %d\n", ((Token *)token->data[i])->len);
  }
}
#endif
