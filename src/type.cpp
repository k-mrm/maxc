#include "type.h"
#include "error.h"
#include "maxc.h"

const char *Type::show() {
    switch(this->type.type) {
    case CTYPE::INT:
        return "int";
    case CTYPE::BOOL:
        return "bool";
    case CTYPE::CHAR:
        return "char";
    case CTYPE::STRING:
        return "string";
    case CTYPE::DOUBLE:
        return "float";
    case CTYPE::LIST:
        return "list";
    case CTYPE::TUPLE:
        return "tuple";
    case CTYPE::FUNCTION:
        return "function";
    case CTYPE::UNINFERRED:
        return "uninferred type";
    case CTYPE::NONE:
        return "none";
    case CTYPE::ANY_VARARG:
        return "any_arg";
    case CTYPE::ANY:
        return "any";
    default:
        error("??????: in type#show");
        return "Error Type";
    }
}

Type *mxcty_none = new Type(CTYPE::NONE);
Type *mxcty_bool = new Type(CTYPE::BOOL);
Type *mxcty_string = new Type(CTYPE::STRING);
Type *mxcty_int = new Type(CTYPE::INT);
Type *mxcty_float = new Type(CTYPE::DOUBLE);

type_t &Type::get() { return this->type; }

bool Type::isnone() { return this->type.type == CTYPE::NONE; }

bool Type::uninfer() { return this->type.type == CTYPE::UNINFERRED; }

bool Type::isint() { return this->type.type == CTYPE::INT; }

bool Type::isfloat() { return this->type.type == CTYPE::DOUBLE; }

bool Type::isstring() { return this->type.type == CTYPE::STRING; }

bool Type::islist() { return this->type.type == CTYPE::LIST; }

bool Type::istuple() { return this->type.type == CTYPE::TUPLE; }

bool Type::isobject() {
    return type.type == CTYPE::STRING || type.type == CTYPE::LIST ||
           type.type == CTYPE::TUPLE;
}

bool Type::isfunction() { return type.type == CTYPE::FUNCTION; }

void Type::tupletype_push(Type *ty) {
    assert(this->type.type == CTYPE::TUPLE);
    this->tuple.push_back(ty);
}
