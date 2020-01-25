enum TKIND {
    TKIND_End,
    TKIND_Num,
    TKIND_String,
    TKIND_Char,
    TKIND_Identifer,
    TKIND_BQLIT,
    // KeyWord
    TKIND_TInt,
    TKIND_TUint,
    TKIND_TInt64,
    TKIND_TUint64,
    TKIND_TBool,
    TKIND_TChar,
    TKIND_TString,
    TKIND_TFloat,
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
    TKIND_Import,
    TKIND_FAILURE,
    TKIND_Break,
    TKIND_Continue,
    TKIND_New,
    TKIND_In,
    TKIND_Null,
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
    TKIND_FatArrow,    // =>
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
    TKIND_Lshift,      // <<
    TKIND_Rshift,      // >>
    TKIND_Assign,      // =
    TKIND_Bang,        // !
    TKIND_Question,    // ?
};
