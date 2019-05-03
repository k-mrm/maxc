#include "maxc.h"

std::string Type::show() {
    switch(this->type.type) {
        case CTYPE::PTR:  return "ptr";
        case CTYPE::INT:  return "int";
        case CTYPE::BOOL:  return "bool";
        case CTYPE::CHAR: return "char";
        case CTYPE::STRING: return "string";
        case CTYPE::LIST: return "list";
        case CTYPE::TUPLE: return "tuple";
        case CTYPE::NONE:   return "none";
        default: error("??????"); return "?????";
    }
}

int Type::get_size() {
    switch(this->type.type) {
        case CTYPE::PTR:  return 8;
        case CTYPE::INT:  return 4;
        case CTYPE::CHAR: return 1;
        default: error("?????"); return -1;
    }
}

type_t Type::get() {
    return this->type;
}

bool Type::islist() {
    return this->type.type == CTYPE::LIST;
}

bool Type::istuple() {
    return this->type.type == CTYPE::TUPLE;
}

bool Type::isobject() {
    return type.type == CTYPE::STRING || type.type == CTYPE::LIST || type.type == CTYPE::TUPLE;
}

bool Type::isfunction() {
    return type.type == CTYPE::FUNCTION;
}

void Type::tupletype_push(Type *ty) {
    assert(this->type.type == CTYPE::TUPLE);
    this->tuple.push_back(ty);
}
