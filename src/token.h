#ifndef MAXC_TOKEN_H
#define MAXC_TOKEN_H

#include "maxc.h"

enum class TKind {
    End,
    Num,
    String,
    Char,
    Identifer,
    // KeyWord
    TInt,
    TUint,
    TInt64,
    TUint64,
    TBool,
    TChar,
    TString,
    TFloat,
    TNone,
    KAnd,
    KOr,
    Struct,
    If,
    Else,
    For,
    While,
    Return,
    Typedef,
    Let,
    Fn,
    True,
    False,
    Const,
    // Symbol
    Lparen,      // (
    Rparen,      // )
    Lbrace,      // {
    Rbrace,      // }
    Lboxbracket, // [
    Rboxbracket, // ]
    Comma,       // ,
    Colon,       // :
    Dot,         // .
    DotDot,      // ..
    Semicolon,   // ;
    Arrow,       // ->
    Inc,         // ++
    Dec,         // --
    Plus,        // +
    Minus,       // -
    Asterisk,    // *
    Div,         // /
    Mod,         // %
    Eq,          // ==
    Neq,         // !=
    Lt,          // <
    Lte,         // <=
    Gt,          // >
    Gte,         // >=
    LogAnd,      // &&
    LogOr,       // ||
    Assign,      // =
    Question,    // ?
};

struct location_t {
    location_t() {}
    location_t(int l, int c) : line(l), col(c) {}

    int line;
    int col;
};

struct token_t {
    TKind type;
    std::string value;

    // for error msg
    location_t start;
    location_t end;
};

class Token {
  public:
    std::vector<token_t> token_v;

    void push_num(std::string &, location_t &, location_t &);
    void push_symbol(std::string &, location_t &, location_t &);
    void push_ident(std::string &, location_t &, location_t &);
    void push_string(std::string &, location_t &, location_t &);
    void push_char(std::string &, location_t &, location_t &);
    void push_end(location_t &, location_t &);

    void show();

    token_t &get();
    token_t &get_step();
    token_t &see(int);
    location_t get_location();
    bool is(TKind);
    bool is(std::string);
    bool isctype();
    bool is_stmt();
    bool skip(TKind);
    bool skip(std::string);
    bool skip2(TKind, TKind);
    bool expect(TKind);
    void step();
    bool step_to(TKind);

    void save();
    void rewind();

  private:
    int save_point;
    bool issaved;
    int pos = 0;
    TKind str2ident(std::string);
    TKind str2symbol(std::string);
    const char *tk2str(TKind);
};

#endif
