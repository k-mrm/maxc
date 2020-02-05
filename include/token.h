#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "util.h"
#include "keyword.h"

typedef struct SrcPos {
    const char *filename;
    int line;
    int col;
} SrcPos;

#define New_SrcPos(f, l, c) ((SrcPos){f, l, c})

typedef struct Token {
    enum TKIND kind;
    int cont;
    char *value;
    uint8_t len;
    /* source position */
    SrcPos start;
    SrcPos end;
} Token;

const char *tk2str(enum TKIND);
void setup_token();
void token_push_num(Vector *, String *, SrcPos, SrcPos);
void token_push_symbol(Vector *, enum TKIND, uint8_t, SrcPos, SrcPos);
void token_push_ident(Vector *, String *, SrcPos, SrcPos);
void token_push_string(Vector *, String *, SrcPos, SrcPos);
void token_push_char(Vector *, char, SrcPos, SrcPos);
void token_push_backquote_lit(Vector *, String *, SrcPos, SrcPos);
void token_push_end(Vector *, SrcPos, SrcPos);
enum TKIND tk_char1(int);
enum TKIND tk_char2(int, int);

#ifdef MXC_DEBUG
void tokendump(Vector *);
#endif

#endif
