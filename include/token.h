#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "util.h"
#include "keyword.h"

enum tkind {
  TKIND_End,
  TKIND_Num,
  TKIND_String,
  TKIND_Char,
  TKIND_Identifer,
  TKIND_BQLIT,
  /* Keyword */
  TKIND_TInt,
  TKIND_TUint,
  TKIND_TInt64,
  TKIND_TUint64,
  TKIND_TBool,
  TKIND_TChar,
  TKIND_TString,
  TKIND_TFloat,
  TKIND_TFile,
  TKIND_TError,
  TKIND_TNone,
  TKIND_KAnd,
  TKIND_KOr,
  TKIND_Struct,
  TKIND_Data,
  TKIND_Object,
  TKIND_If,
  TKIND_Else,
  TKIND_For,
  TKIND_While,
  TKIND_Return,
  TKIND_Typedef,
  TKIND_Let,
  TKIND_Fn,
  TKIND_True,
  TKIND_False,
  TKIND_Const,
  TKIND_Use,
  TKIND_FAILURE,
  TKIND_Break,
  TKIND_Skip,
  TKIND_New,
  TKIND_In,
  TKIND_Null,
  TKIND_Xor,
  TKIND_BreakPoint,
  TKIND_Assert,
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
};

typedef struct SrcPos {
  const char *filename;
  int line;
  int col;
} SrcPos;

#define New_SrcPos(f, l, c) ((SrcPos){f, l, c})

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
