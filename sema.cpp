#include "maxc.h"

Ast_v &SemaAnalyzer::run(Ast_v &ast) {
    for(Ast *a: ast)
        ret_ast.push_back(visit(a));

    return ret_ast;
}

Ast *SemaAnalyzer::visit(Ast *ast) {
    if(ast == nullptr) return nullptr;

    switch(ast->get_nd_type()) {
        case NDTYPE::NUM:
        case NDTYPE::BOOL:
        case NDTYPE::CHAR:
        case NDTYPE::STRING:
            return ast;
        case NDTYPE::LIST:
        case NDTYPE::SUBSCR:
        case NDTYPE::TUPLE:
            break;
        case NDTYPE::BINARY:
            return visit_binary(ast);
        case NDTYPE::DOT:
        case NDTYPE::UNARY:
        case NDTYPE::TERNARY:
            break;
        case NDTYPE::ASSIGNMENT:
            return visit_assign(ast);
        case NDTYPE::IF:
        case NDTYPE::FOR:
        case NDTYPE::WHILE:
        case NDTYPE::BLOCK:
        case NDTYPE::PRINT:
        case NDTYPE::PRINTLN:
        case NDTYPE::RETURN:
            break;
        case NDTYPE::VARIABLE:
            return visit_load(ast);
        case NDTYPE::FUNCCALL:
            return visit_fncall(ast);
        case NDTYPE::FUNCDEF:
            break;
        case NDTYPE::VARDECL:
            return visit_vardecl(ast);
        default:    error("internal error in SemaAnalyzer");
    }

    return nullptr;
}

Ast *SemaAnalyzer::visit_binary(Ast *ast) {
    auto b = (NodeBinop *)ast;

    b->left = visit(b->left);
    b->right = visit(b->right);

    b->ctype = checktype(b->left->ctype, b->right->ctype);

    return b;
}

Ast *SemaAnalyzer::visit_assign(Ast *ast) {
    auto a = (NodeAssignment *)ast;

    if(a->dst->get_nd_type() != NDTYPE::VARIABLE &&
       a->dst->get_nd_type() != NDTYPE::SUBSCR) {
        error("left side of the expression is not valid");
    }

    NodeVariable *v = (NodeVariable *)a->dst;
    //subscr?

    if(v->vinfo.vattr & (int)VarAttr::Const) {
        error("assignment of read-only variable");
    }

    a->src = visit(a->src);

    v->vinfo.vattr &= ~((int)VarAttr::Uninit);

    checktype(a->dst->ctype, a->src->ctype);

    return a;
}

Ast *SemaAnalyzer::visit_vardecl(Ast *ast) {
    auto v = (NodeVardecl *)ast;

    if(v->init != nullptr) {
        v->init = visit(v->init);

        checktype(v->var->ctype, v->init->ctype);
    }
    else {
        v->var->vinfo.vattr |= (int)VarAttr::Uninit;
    }

    return v;
}

Ast *SemaAnalyzer::visit_fncall(Ast *ast) {
    auto f = (NodeFnCall *)ast;

    f->func = (NodeVariable *)visit(f->func);

    NodeVariable *fn = (NodeVariable *)f->func;

    if(fn->finfo.isbuiltin) {
        return visit_bltinfn_call(f);
    }

    if(!fn->ctype->isfunction()) {
        error("must be function");
    }

    if(f->args.size() != fn->finfo.ftype->fnarg.size()) {
        error("bad arg");
    }

    int n = 0;
    for(auto a: f->args) {
        a = visit(a);
        checktype(a->ctype, fn->finfo.ftype->fnarg[n]);
        ++n;
    }

    f->ctype = fn->finfo.ftype->fnret;

    return f;
}

Ast *SemaAnalyzer::visit_bltinfn_call(NodeFnCall *fn) {
    ;
}

Ast *SemaAnalyzer::visit_load(Ast *ast) {
    auto v = (NodeVariable *)ast;

    if(v->vinfo.vattr & (int)VarAttr::Uninit) {
        error("use of uninit variable");
    }

    return v;
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
