#include "maxc.h"

std::string Type::show() {
    switch(this->type.type) {
        case TYPE_INT:  return "int";
        case TYPE_VOID: return "void";
        default:        return "?????";
    }
}

int Type::get_size() {
    switch(this->type.type) {
        case TYPE_INT:  return 4;
        case TYPE_VOID: return 0;
        default:        return -1;
    }
}
