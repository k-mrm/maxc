#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "maxc.h"
#include "util.h"
#include "keyword.h"

typedef struct Location {
    int line;
    int col;
} Location;

#define New_Location(l, c) ((Location){l, c})

typedef struct Token {
    enum TKIND kind;
    // Token kind
    int cont;
    // kind == TKIND_BQLIT
    char *value;
    // token's value(string)
    uint8_t len;
    // length of token

    Location start;
    Location end;
} Token;

const char *tk2str(enum TKIND);
void setup_token();
void token_push_num(Vector *, String *, Location, Location);
void token_push_symbol(Vector *, enum TKIND, uint8_t, Location, Location);
void token_push_ident(Vector *, String *, Location, Location);
void token_push_string(Vector *, String *, Location, Location);
void token_push_backquote_lit(Vector *, String *, Location, Location);
void token_push_end(Vector *, Location, Location);
enum TKIND tk_char1(int);
enum TKIND tk_char2(int, int);

#ifdef MXC_DEBUG
void tokendump(Vector *);
#endif

#endif
