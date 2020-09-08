#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "util.h"

enum tkind {
  TKIND_End,
  TKIND_Num,
  TKIND_String,
  TKIND_Char,
  TKIND_Identifer,
  TKIND_BQLIT,
  /* Keyword */
#define KEYWORD(_, k) k,
#define KEYWORD_ALIAS(a, b)
#include "keyword.h"
  TKIND_TUint,
  TKIND_TInt64,
  TKIND_TUint64,
  TKIND_TChar,
#undef KEYWORD
#undef KEYWORD_ALIAS
  /* Symbol */
  TKIND_Lparen,      /* (  */
  TKIND_Rparen,      /* )  */
  TKIND_Lbrace,      /* {  */
  TKIND_Rbrace,      /* }  */
  TKIND_Lboxbracket, /* [  */
  TKIND_Rboxbracket, /* ]  */
  TKIND_Comma,       /* ,  */
  TKIND_Colon,       /* :  */
  TKIND_Dot,         /* .  */
  TKIND_DotDot,      /* .. */
  TKIND_Semicolon,   /* ;  */
  TKIND_Arrow,       /* -> */
  TKIND_FatArrow,    /* => */
  TKIND_Inc,         /* ++ */
  TKIND_Dec,         /* -- */
  TKIND_Plus,        /* +  */
  TKIND_Minus,       /* -  */
  TKIND_Asterisk,    /* *  */
  TKIND_Div,         /* /  */
  TKIND_Mod,         /* %  */
  TKIND_PlusAs,      /* += */
  TKIND_MinusAs,     /* -= */
  TKIND_AsteriskAs,  /* *= */
  TKIND_DivAs,       /* /= */
  TKIND_ModAs,       /* %= */
  TKIND_Eq,          /* == */
  TKIND_Neq,         /* != */
  TKIND_Lt,          /* <  */
  TKIND_Lte,         /* <= */
  TKIND_Gt,          /* >  */
  TKIND_Gte,         /* >= */
  TKIND_LogAnd,      /* && */
  TKIND_LogOr,       /* || */
  TKIND_Lshift,      /* << */
  TKIND_Rshift,      /* >> */
  TKIND_Assign,      /* =  */
  TKIND_Bang,        /* !  */
  TKIND_Question,    /* ?  */
  TKIND_Atmark,      /* @  */
  TKIND_Hash,        /* #  */
};

typedef struct SrcPos {
  const char *filename;
  int line;
  int col;
} SrcPos;

#define cur_srcpos(f, l, c) ((SrcPos){f, l, c})

typedef struct Token {
  enum tkind kind;
  int cont;
  char *value;
  uint8_t len;
  /* source position */
  SrcPos start;
  SrcPos end;
} Token;

const char *tk2str(enum tkind);
void setup_token();
void token_push_num(Vector *, String *, SrcPos, SrcPos);
void token_push_symbol(Vector *, enum tkind, uint8_t, SrcPos, SrcPos);
void token_push_ident(Vector *, String *, SrcPos, SrcPos);
void token_push_string(Vector *, String *, SrcPos, SrcPos);
void token_push_char(Vector *, char, SrcPos, SrcPos);
void token_push_backquote_lit(Vector *, String *, SrcPos, SrcPos);
void token_push_end(Vector *, SrcPos, SrcPos);
enum tkind tk_char1(int);
enum tkind tk_char2(int, int);

#ifdef MXC_DEBUG
void tokendump(Vector *);
#endif

#endif
