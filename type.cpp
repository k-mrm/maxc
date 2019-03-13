#include "maxc.h"

std::string Type::show() {
    switch(this->type.type) {
        case CTYPE::PTR:  return "ptr";
        case CTYPE::INT:  return "int";
        case CTYPE::CHAR: return "char";
        case CTYPE::STRING: return "string";
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
