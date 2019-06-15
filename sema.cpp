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
            break;
        case NDTYPE::STRING:
            break;
        case NDTYPE::LIST:
        case NDTYPE::SUBSCR:
        case NDTYPE::TUPLE:
            break;
        case NDTYPE::BINARY:
            visit_binary(ast); break;
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
            break;
        default:    error("internal error in SemaAnalyzer");
    }
}

void SemaAnalyzer::visit_binary(Ast *ast) {
    auto b = (NodeBinop *)ast;

    visit(b->left);
    visit(b->right);

    b->ctype = checktype(b->left->ctype, b->right->ctype);
}

Type *SemaAnalyzer::checktype(Type *ty1, Type *ty2) {
    if(ty1 == nullptr || ty2 == nullptr) return nullptr;

    bool swapped = false;

    if(ty1->islist()) {
        if(!ty2->islist()) goto err;
        Type *b = ty1;

        for(;;) {
            ty1 = ty1->ptr;
            ty2 = ty2->ptr;

            if(ty1 == nullptr && ty2 == nullptr) return b;
            if(ty1 == nullptr || ty2 == nullptr) goto err;
            checktype(ty1, ty2);
        }
    }
    else if(ty1->istuple()) {
        if(!ty2->istuple()) goto err;
        if(ty1->tuple.size() != ty2->tuple.size()) goto err;
        int s = ty1->tuple.size();
        int cnt = 0;

        for(;;) {
            checktype(ty1->tuple[cnt], ty2->tuple[cnt]);
            ++cnt;
            if(cnt == s) return ty1;
        }
    }
    else if(ty1->isfunction()) {
        if(!ty2->isfunction()) goto err;
        if(ty1->fnarg.size() != ty2->fnarg.size()) goto err;
        if(ty1->fnret->get().type != ty1->fnret->get().type) goto err;

        int i = ty1->fnarg.size(); int cnt = 0;

        if(i == 0) return ty1;

        for(;;) {
            checktype(ty1->fnarg[cnt], ty2->fnarg[cnt]);
            ++cnt;
            if(cnt == i) return ty1;
        }
    }

    if(ty1->get().type == ty2->get().type) return ty1;

    if(ty1->get().type > ty2->get().type) {
        std::swap(ty1, ty2);
        swapped = true;
    }

    switch(ty1->get().type) {
        case CTYPE::NONE:
            goto err;
        case CTYPE::INT:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else if(ty2->get().type == CTYPE::UINT || ty2->get().type == CTYPE::INT64 ||
                    ty2->get().type == CTYPE::UINT64)
                return ty2;
            else
                goto err;
        case CTYPE::UINT:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else if(ty2->get().type == CTYPE::INT64)
                return ty2;
            else if(ty2->get().type == CTYPE::UINT64)
                return ty2;
            else
                goto err;
        case CTYPE::UINT64:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else goto err;
        case CTYPE::BOOL:
        case CTYPE::CHAR:
        case CTYPE::STRING:
        case CTYPE::LIST:
                goto err;
        default:
            error("unimplemented(check type)"); return nullptr;
    }

err:
    if(swapped) std::swap(ty1, ty2);

    /*
    error(token.see(-1).line, token.see(-1).col,
            "expected type `%s`, found type `%s`",
            ty1->show().c_str(), ty2->show().c_str());*/ //TODO
    error("bad type");

    return nullptr;
}
