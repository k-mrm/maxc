#include "maxc.h"

std::string Type::show() {
    switch(this->type.type) {
        case CTYPE::INT:  return "int";
        case CTYPE::CHAR: return "char";
        case CTYPE::VOID: return "void";
        default:        return "?????";
    }
}

int Type::get_size() {
    switch(this->type.type) {
        case CTYPE::INT:  return 4;
        case CTYPE::CHAR: return 1;
        case CTYPE::VOID: return 0;
        default:        return -1;
    }
}

type_t Type::get() {
    return this->type;
}
