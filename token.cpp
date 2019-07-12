#include "token.h"
#include "error.h"
#include "maxc.h"

void Token::push_num(std::string &value, location_t &start, location_t &end) {
    token_v.push_back((token_t){TKind::Num, value, start, end});
}

void Token::push_symbol(std::string &value, location_t &start,
                        location_t &end) {
    TKind skind = str2symbol(value);
    token_v.push_back((token_t){skind, value, start, end});
}

void Token::push_ident(std::string &value, location_t &start, location_t &end) {
    TKind ikind = str2ident(value);
    token_v.push_back((token_t){ikind, value, start, end});
}

void Token::push_string(std::string &value, location_t &start,
                        location_t &end) {
    token_v.push_back((token_t){TKind::String, value, start, end});
}

void Token::push_char(std::string &value, location_t &start, location_t &end) {
    token_v.push_back((token_t){TKind::Char, value, start, end});
}

void Token::push_end(location_t &start, location_t &end) {
    token_v.push_back((token_t){TKind::End, "", start, end});
}

token_t Token::get() { return token_v[pos]; }

token_t Token::get_step() { return token_v[pos++]; }

token_t &Token::see(int p) { return token_v[pos + p]; }

bool Token::is(TKind tk) { return token_v[pos].type == tk; }

bool Token::is(std::string val) { return token_v[pos].value == val; }

bool Token::isctype() {
    if(is(TKind::TInt) || is(TKind::TChar) || is(TKind::TString))
        return true;
    else
        return false;
}

bool Token::is_stmt() {
    if(is(TKind::For) || is(TKind::Fn) || is(TKind::Let) || is(TKind::While) ||
       is(TKind::Return) || is(TKind::Lbrace))
        return true;
    else
        return false;
}

void Token::step() { pos++; }

bool Token::skip(TKind tk) {
    if(token_v[pos].type == tk) {
        ++pos;
        return true;
    }
    else
        return false;
}

bool Token::skip(std::string val) {
    if(token_v[pos].value == val) {
        ++pos;
        return true;
    }
    else
        return false;
}

bool Token::skip2(TKind v1, TKind v2) {
    save();
    if(token_v[pos].type == v1) {
        ++pos;
        if(token_v[pos].type == v2) {
            ++pos;
            return true;
        }
    }
    rewind();
    return false;
}

bool Token::expect(TKind tk) {
    if(token_v[pos].type == tk) {
        pos++;
        return true;
    }
    else {
        error(token_v[pos].start, token_v[pos].end, "expected token ` %s `",
              tk2str(tk));
        return false;
    }
}

bool Token::step_to(TKind tk) { return token_v[pos++].type == tk; }

void Token::save() {
    save_point = pos;
    issaved = true;
}

void Token::rewind() {
    if(issaved) {
        pos = save_point;
        issaved = false;
    }
    else
        puts("[warning] you don't save");
}

void Token::show() {
    /*
    std::string literal;

    for(token_t token: token_v) {
        literal = [&]() -> std::string {
            if(token.type == TOKEN_TYPE::NUM)
                return "Number";
            else if(token.type == TOKEN_TYPE::SYMBOL)
                return "Symbol";
            else if(token.type == TOKEN_TYPE::IDENTIFER)
                return "Identifer";
            else if(token.type == TOKEN_TYPE::STRING)
                return "String";
            else if(token.type == TOKEN_TYPE::CHAR)
                return "Char";
            else if(token.type == TOKEN_TYPE::END)
                return "End";
            else {
                printf("???");
                exit(1);
            }
        }();

        TODO TODO TODO         TODO        TODO TODO               TODO
             TODO          TODO    TODO    TODO    TODO        TODO    TODO
             TODO         TODO      TODO   TODO      TODO     TODO      TODO
             TODO        TODO        TODO  TODO       TODO   TODO        TODO
             TODO        TODO        TODO  TODO       TODO   TODO        TODO
             TODO         TODO      TODO   TODO      TODO     TODO      TODO
             TODO          TODO    TODO    TODO    TODO        TODO    TODO
             TODO              TODO        TODO TODO               TODO

        std::cout << "start "<< token.start << ":end " << token.end <<  ": "
            << literal << "( " << token.value << " )" << std::endl;
    }
    */
}

TKind Token::str2ident(std::string ident) {
    if(ident == "int")
        return TKind::TInt;
    if(ident == "uint")
        return TKind::TUint;
    if(ident == "int64")
        return TKind::TInt64;
    if(ident == "uint64")
        return TKind::TUint64;
    if(ident == "bool")
        return TKind::TBool;
    if(ident == "char")
        return TKind::TChar;
    if(ident == "string")
        return TKind::TString;
    if(ident == "none")
        return TKind::TNone;
    if(ident == "or")
        return TKind::KOr;
    if(ident == "and")
        return TKind::KAnd;
    if(ident == "if")
        return TKind::If;
    if(ident == "else")
        return TKind::Else;
    if(ident == "for")
        return TKind::For;
    if(ident == "while")
        return TKind::While;
    if(ident == "return")
        return TKind::Return;
    if(ident == "typedef")
        return TKind::Typedef;
    if(ident == "let")
        return TKind::Let;
    if(ident == "fn")
        return TKind::Fn;
    if(ident == "true")
        return TKind::True;
    if(ident == "false")
        return TKind::False;
    if(ident == "const")
        return TKind::Const;
    return TKind::Identifer;
}

TKind Token::str2symbol(std::string sym) {
    if(sym == "(")
        return TKind::Lparen;
    if(sym == ")")
        return TKind::Rparen;
    if(sym == "{")
        return TKind::Lbrace;
    if(sym == "}")
        return TKind::Rbrace;
    if(sym == "[")
        return TKind::Lboxbracket;
    if(sym == "]")
        return TKind::Rboxbracket;
    if(sym == ",")
        return TKind::Comma;
    if(sym == ":")
        return TKind::Colon;
    if(sym == ".")
        return TKind::Dot;
    if(sym == "..")
        return TKind::DotDot;
    if(sym == ";")
        return TKind::Semicolon;
    if(sym == "->")
        return TKind::Arrow;
    if(sym == "++")
        return TKind::Inc;
    if(sym == "--")
        return TKind::Dec;
    if(sym == "+")
        return TKind::Plus;
    if(sym == "-")
        return TKind::Minus;
    if(sym == "*")
        return TKind::Asterisk;
    if(sym == "/")
        return TKind::Div;
    if(sym == "%")
        return TKind::Mod;
    if(sym == "==")
        return TKind::Eq;
    if(sym == "!=")
        return TKind::Neq;
    if(sym == "<")
        return TKind::Lt;
    if(sym == "<=")
        return TKind::Lte;
    if(sym == ">")
        return TKind::Gt;
    if(sym == ">=")
        return TKind::Gte;
    if(sym == "&&")
        return TKind::LogAnd;
    if(sym == "||")
        return TKind::LogOr;
    if(sym == "=")
        return TKind::Assign;
    if(sym == "?")
        return TKind::Question;
    error("internal error");
    return (TKind)-1;
}

const char *Token::tk2str(TKind tk) {
    switch(tk) {
    case TKind::End:
        return "End";
    case TKind::Num:
        return "Number";
    case TKind::String:
        return "String";
    case TKind::Char:
        return "Char";
    case TKind::Identifer:
        return "Identifer";
    case TKind::TInt:
        return "int";
    case TKind::TUint:
        return "uint";
    case TKind::TInt64:
        return "int64";
    case TKind::TUint64:
        return "uint64";
    case TKind::TBool:
        return "bool";
    case TKind::TChar:
        return "char";
    case TKind::TString:
        return "string";
    case TKind::TNone:
        return "none";
    case TKind::KAnd:
        return "and";
    case TKind::KOr:
        return "or";
    case TKind::If:
        return "if";
    case TKind::Else:
        return "else";
    case TKind::For:
        return "for";
    case TKind::While:
        return "while";
    case TKind::Return:
        return "return";
    case TKind::Typedef:
        return "typedef";
    case TKind::Let:
        return "let";
    case TKind::Fn:
        return "fn";
    case TKind::True:
        return "true";
    case TKind::False:
        return "false";
    case TKind::Const:
        return "const";
    case TKind::Lparen:
        return "(";
    case TKind::Rparen:
        return ")";
    case TKind::Lbrace:
        return "{";
    case TKind::Rbrace:
        return "}";
    case TKind::Lboxbracket:
        return "[";
    case TKind::Rboxbracket:
        return "]";
    case TKind::Comma:
        return ",";
    case TKind::Colon:
        return ":";
    case TKind::Dot:
        return ".";
    case TKind::DotDot:
        return "..";
    case TKind::Semicolon:
        return ";";
    case TKind::Arrow:
        return "->";
    case TKind::Inc:
        return "++";
    case TKind::Dec:
        return "--";
    case TKind::Plus:
        return "+";
    case TKind::Minus:
        return "-";
    case TKind::Asterisk:
        return "*";
    case TKind::Div:
        return "/";
    case TKind::Mod:
        return "%";
    case TKind::Eq:
        return "==";
    case TKind::Neq:
        return "!=";
    case TKind::Lt:
        return "<";
    case TKind::Lte:
        return "<=";
    case TKind::Gt:
        return ">";
    case TKind::Gte:
        return ">=";
    case TKind::LogAnd:
        return "&&";
    case TKind::LogOr:
        return "||";
    case TKind::Assign:
        return "=";
    case TKind::Question:
        return "?";
    default:
        return "error";
    }
    return "error";
}
