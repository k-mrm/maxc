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
#define KEYWORD(s, k) {s, k},
#define KEYWORD_ALIAS(s, k) {s, k},
#include "keyword.h"
#undef KEYWORD
#undef KEYWORD_ALIAS
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
    case '#': return TKIND_Hash;
    default:
      panic("unknown symbol: %c", c);
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
    case '.':
      switch(c2) {
        case '.':
          return TKIND_DotDot;
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
#define KEYWORD(s, k) case k: return s;
#define KEYWORD_ALIAS(s, k)
#include "keyword.h"
#undef KEYWORD
#undef KEYWORD_ALIAS
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

static Token *newtoken(enum tkind kind,
    char *value,
    uint8_t len,
    SrcPos s,
    SrcPos e) {
  Token *self = malloc(sizeof(Token));

  self->kind = kind;
  self->value = value;
  self->len = len;
  self->start = s;
  self->end = e;

  return self;
}

static Token *newtoken_ch(char c, SrcPos s, SrcPos e) {
  Token *self = malloc(sizeof(Token));
  self->kind = TKIND_Char;
  self->cont = c;
  self->len = 1;
  self->start = s;
  self->end = e;

  return self;
}

static Token *newtoken_bq(
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

static Token *newtoken_sym(enum tkind kind,
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

static Token *newtoken_end(SrcPos s, SrcPos e) {
  Token *self = malloc(sizeof(Token));

  self->kind = TKIND_End;
  self->value = "";
  self->len = 1;
  self->start = s;
  self->end = e;

  return self;
}

void token_push_num(Vector *self, char *value, uint8_t len, SrcPos s, SrcPos e) {
  Token *tk = newtoken(TKIND_Num, value, len, s, e);

  vec_push(self, tk);
}

void token_push_ident(Vector *self, char *val, uint8_t len, SrcPos s, SrcPos e) {
  enum tkind kind = ident2kw(val);

  Token *tk = newtoken(kind, val, len, s, e);

  vec_push(self, tk);
}

void token_push_symbol(
    Vector *self,
    enum tkind kind,
    uint8_t len,
    SrcPos s,
    SrcPos e
    ) {
  Token *tk = newtoken_sym(kind, len, s, e);

  vec_push(self, tk);
}

void token_push_string(Vector *self, char *str, uint8_t len, SrcPos s, SrcPos e) {
  Token *tk = newtoken(TKIND_String, str, len, s, e);

  vec_push(self, tk);
}

void token_push_char(Vector *self, char c, SrcPos s, SrcPos e) {
  vec_push(self, newtoken_ch(c, s, e));
}

void token_push_backquote_lit(Vector *self, char *str, uint8_t len, SrcPos s, SrcPos e) {
  enum tkind a = 0;

  if(len == 1) {
    a = op_char1(str[0]);
  }
  else if(len == 2) {
    a = op_char2(str[0], str[1]);
  }

  Token *tk = newtoken_bq(a, len, s, e);

  vec_push(self, tk);
}

void token_push_end(Vector *self, SrcPos s, SrcPos e) {
  Token *tk = newtoken_end(s, e);

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
