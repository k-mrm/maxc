#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "maxc.h"
#include "util.h"

enum TKIND {
    TKIND_End,
    TKIND_Num,
    TKIND_String,
    TKIND_Char,
    TKIND_Identifer,
    // KeyWord
    TKIND_TInt,
    TKIND_TUint,
    TKIND_TInt64,
    TKIND_TUint64,
    TKIND_TBool,
    TKIND_TChar,
    TKIND_TString,
    TKIND_TFloat,
    TKIND_TNone,
    TKIND_KAnd,
    TKIND_KOr,
    TKIND_Struct,
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
    // Symbol
    TKIND_Lparen,      // (
    TKIND_Rparen,      // )
    TKIND_Lbrace,      // {
    TKIND_Rbrace,      // }
    TKIND_Lboxbracket, // [
    TKIND_Rboxbracket, // ]
    TKIND_Comma,       // ,
    TKIND_Colon,       // :
    TKIND_Dot,         // .
    TKIND_DotDot,      // ..
    TKIND_Semicolon,   // ;
    TKIND_Arrow,       // ->
    TKIND_Inc,         // ++
    TKIND_Dec,         // --
    TKIND_Plus,        // +
    TKIND_Minus,       // -
    TKIND_Asterisk,    // *
    TKIND_Div,         // /
    TKIND_Mod,         // %
    TKIND_PlusAs,      // +=
    TKIND_MinusAs,     // -=
    TKIND_AsteriskAs,  // *=
    TKIND_DivAs,       // /=
    TKIND_ModAs,       // %=
    TKIND_Eq,          // ==
    TKIND_Neq,         // !=
    TKIND_Lt,          // <
    TKIND_Lte,         // <=
    TKIND_Gt,          // >
    TKIND_Gte,         // >=
    TKIND_LogAnd,      // &&
    TKIND_LogOr,       // ||
    TKIND_Assign,      // =
    TKIND_Exclamation, // !
    TKIND_Question,    // ?
};

typedef struct Location {
    int line;
    int col;
} Location;

#define New_Location(l, c) ((Location){l, c})

typedef struct Token {
    enum TKIND kind;
    //Token kind
    char *value;
    //token's value(string)
    uint8_t len;
    //length of token

    Location start;
    Location end;
} Token;

const char *tk2str(enum TKIND);
void setup_token();
void token_push_num(Vector *, String *, Location, Location);
void token_push_symbol(Vector *, enum TKIND, uint8_t, Location, Location);
void token_push_ident(Vector *, String *, Location, Location);
void token_push_string(Vector *, String *, Location, Location);
void token_push_end(Vector *, Location, Location);
enum TKIND tk_char1(int);
enum TKIND tk_char2(int, int);

#ifdef MXC_DEBUG
void tokendump(Vector *);
#endif

#endif
