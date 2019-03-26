#include"maxc.h"

void Token::push_num(std::string value, int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::NUM, value, line, col});
}

void Token::push_symbol(std::string value, int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::SYMBOL, value, line, col});
}

void Token::push_ident(std::string value, int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::IDENTIFER, value, line, col});
}

void Token::push_string(std::string value, int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::STRING, value, line, col});
}

void Token::push_char(std::string value, int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::CHAR, value, line, col});
}

void Token::push_end(int line, int col) {
    token_v.push_back((token_t){TOKEN_TYPE::END, "", line, col});
}

token_t Token::get() {
    return token_v[pos];
}

token_t Token::get_step() {
    return token_v[pos++];
}

token_t Token::see(int p) {
    return token_v[pos + p];
}

bool Token::is_value(std::string tk) {
    return token_v[pos].value == tk;
}

bool Token::is_type(TOKEN_TYPE ty) {
    return token_v[pos].type == ty;
}

bool Token::isctype() {
    if(is_value("int") || is_value("char") || is_value("string"))
        return true;
    else return false;
}

bool Token::is_stmt() {
    if(is_type(TOKEN_TYPE::IDENTIFER) && (is_value("for") || is_value("fn") || is_value("let") ||
       is_value("while") ||is_value("return") || is_value("print") || is_value("println") ||
       is_value("{"))) return true;
    else return false;
}

void Token::step() {
    pos++;
}

bool Token::skip(std::string val) {
    if(token_v[pos].value == val) {
        pos++;
        return true;
    }
    else
        return false;
}

bool Token::expect(std::string val) {
    if(token_v[pos].value == val) {
        pos++;
        return true;
    }
    else {
        error(token_v[pos].line, token_v[pos].col, "expected token ` %s `", val.c_str());
        while(!step_to(";"));
        return false;
    }
}

bool Token::step_to(std::string val) {
   return token_v[pos++].value == val;
}

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

        std::cout << "line "<< token.line << ":col " << token.col <<  ": "
            << literal << "( " << token.value << " )" << std::endl;
    }
}

