#include "ast.h"
#include "error.h"

bool Ast::isexpr() {
    switch(this->get_nd_type()) {
    case NDTYPE::NUM:
    case NDTYPE::BOOL:
    case NDTYPE::CHAR:
    case NDTYPE::LIST:
    case NDTYPE::SUBSCR:
    case NDTYPE::TUPLE:
    case NDTYPE::FUNCCALL:
    case NDTYPE::ASSIGNMENT:
    case NDTYPE::VARIABLE:
    case NDTYPE::STRING:
    case NDTYPE::BINARY:
    case NDTYPE::MEMBER:
    case NDTYPE::UNARY:
    case NDTYPE::TERNARY:
        return true;
    default:
        return false;
    }
}
