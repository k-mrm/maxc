#include "maxc.h"

Ast_v &SemaAnalyzer::run(Ast_v &ast) {
    for(Ast *a: ast)
        visit(a);

    return ast;
}

void SemaAnalyzer::visit(Ast *ast) {
    if(ast == nullptr) return;

    switch(ast->get_nd_type()) {
        case NDTYPE::NUM:
        case NDTYPE::BOOL:
        case NDTYPE::CHAR:
        case NDTYPE::STRING:
        case NDTYPE::LIST:
        case NDTYPE::SUBSCR:
        case NDTYPE::TUPLE:
        case NDTYPE::BINARY:
        case NDTYPE::DOT:
        case NDTYPE::UNARY:
        case NDTYPE::TERNARY:
        case NDTYPE::ASSIGNMENT:
        case NDTYPE::IF:
        case NDTYPE::FOR:
        case NDTYPE::WHILE:
        case NDTYPE::BLOCK:
        case NDTYPE::PRINT:
        case NDTYPE::PRINTLN:
        case NDTYPE::RETURN:
        case NDTYPE::VARIABLE:
        case NDTYPE::FUNCCALL:
        case NDTYPE::FUNCDEF:
        case NDTYPE::VARDECL:
        default:    error("internal error in SemaAnalyzer");
    }
}
