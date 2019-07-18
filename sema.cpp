#include "sema.h"
#include "error.h"
#include "maxc.h"

Ast_v &SemaAnalyzer::run(Ast_v &ast) {
    for(Ast *a : ast)
        ret_ast.push_back(visit(a));
    return ret_ast;
}

Ast *SemaAnalyzer::visit(Ast *ast) {
    if(ast == nullptr)
        return nullptr;

    switch(ast->get_nd_type()) {
    case NDTYPE::NUM:
    case NDTYPE::BOOL:
    case NDTYPE::CHAR:
    case NDTYPE::STRING:
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
        return visit_if(ast);
    case NDTYPE::FOR:
        break;
    case NDTYPE::WHILE:
        return visit_while(ast);
    case NDTYPE::BLOCK:
        return visit_block(ast);
    case NDTYPE::RETURN:
        return visit_return(ast);
    case NDTYPE::VARIABLE:
        return visit_load(ast);
    case NDTYPE::FUNCCALL:
        return visit_fncall(ast);
    case NDTYPE::FUNCDEF:
        return visit_funcdef(ast);
    case NDTYPE::VARDECL:
        return visit_vardecl(ast);
    default:
        error("internal error in SemaAnalyzer");
    }

    return ast;
}

Ast *SemaAnalyzer::visit_binary(Ast *ast) {
    auto b = (NodeBinop *)ast;

    b->left = visit(b->left);
    b->right = visit(b->right);

    if(b->op == "<" || b->op == ">" || b->op == "<=" ||
       b->op == ">=" || b->op == "!=" || b->op == "==") {
        checktype(b->left->ctype, b->right->ctype);

        b->ctype = new Type(CTYPE::BOOL);
    }
    else {
        b->ctype = checktype(b->left->ctype, b->right->ctype);
        b->left->ctype = b->ctype;
        b->right->ctype = b->ctype;
    }

    return b;
}

Ast *SemaAnalyzer::visit_assign(Ast *ast) {
    auto a = (NodeAssignment *)ast;

    if(a->dst->get_nd_type() != NDTYPE::VARIABLE &&
       a->dst->get_nd_type() != NDTYPE::SUBSCR) {
        error("left side of the expression is not valid");
    }

    NodeVariable *v = (NodeVariable *)a->dst;
    // TODO: subscr?

    if(v->vinfo.vattr & (int)VarAttr::Const) {
        error("assignment of read-only variable");
    }

    a->src = visit(a->src);

    v->vinfo.vattr &= ~((int)VarAttr::Uninit);

    checktype(a->dst->ctype, a->src->ctype);

    return a;
}

Ast *SemaAnalyzer::visit_block(Ast *ast) {
    auto b = (NodeBlock *)ast;

    for(auto &a: b->cont) {
        a = visit(a);
    }

    return b;
}

Ast *SemaAnalyzer::visit_if(Ast *ast) {
    auto i = (NodeIf *)ast;

    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    return i;
}

Ast *SemaAnalyzer::visit_while(Ast *ast) {
    auto w = (NodeWhile *)ast;

    w->cond = visit(w->cond);
    w->body = visit(w->body);

    return w;
}

Ast *SemaAnalyzer::visit_return(Ast *ast) {
    auto r = (NodeReturn *)ast;

    r->cont = visit(r->cont);

    if(fn_saver.empty()) {
        error("use of return statement outside function");
    }
    else {
        Type *cur_fn_retty = fn_saver.top()->finfo.ftype->fnret;

        if(!checktype(cur_fn_retty, r->cont->ctype)) {
            error("return type error");
        }
    }

    return r;
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
    for(auto a : f->args) {
        a = visit(a);
        checktype(a->ctype, fn->finfo.ftype->fnarg[n]);
        ++n;
    }

    f->ctype = fn->finfo.ftype->fnret;

    return f;
}

Ast *SemaAnalyzer::visit_funcdef(Ast *ast) {
    NodeFunction *fn = (NodeFunction *)ast;

    fn_saver.push(fn);

    for(auto &a : fn->block) {
        a = visit(a);
    }

    fn_saver.pop();

    return fn;
}

Ast *SemaAnalyzer::visit_bltinfn_call(NodeFnCall *f) {
    NodeVariable *fn = (NodeVariable *)f->func;

    for(auto a : f->args) {
        a = visit(a);
    }

    switch(fn->finfo.fnkind) {
    case BltinFnKind::Print:
        f->ctype = new Type(CTYPE::NONE);
        break;
    case BltinFnKind::Println:
        f->ctype = new Type(CTYPE::NONE);
        break;
    default:
        f->ctype = nullptr;
        error("unimplemented: in visit_bltinfn_call");
    }
    // TODO: about arguments

    return f;
}

Ast *SemaAnalyzer::visit_load(Ast *ast) {
    auto v = (NodeVariable *)ast;

    /*
    if(v->vinfo.vattr & (int)VarAttr::Uninit) {
        if(v->ctype->isfunction())
            debug("load %s\n", v->finfo.name.c_str());
        else
            debug("load %s\n", v->vinfo.name.c_str());
        error("use of uninit variable");
    }*/

    return v;
}

Type *SemaAnalyzer::checktype(Type *ty1, Type *ty2) {
    if(ty1 == nullptr || ty2 == nullptr)
        return nullptr;

    if(ty1->islist()) {
        if(!ty2->islist())
            goto err;
        Type *b = ty1;

        for(;;) {
            ty1 = ty1->ptr;
            ty2 = ty2->ptr;

            if(ty1 == nullptr && ty2 == nullptr)
                return b;
            if(ty1 == nullptr || ty2 == nullptr)
                goto err;
            checktype(ty1, ty2);
        }
    }
    else if(ty1->istuple()) {
        if(!ty2->istuple())
            goto err;
        if(ty1->tuple.size() != ty2->tuple.size())
            goto err;
        int s = ty1->tuple.size();
        int cnt = 0;

        for(;;) {
            checktype(ty1->tuple[cnt], ty2->tuple[cnt]);
            ++cnt;
            if(cnt == s)
                return ty1;
        }
    }
    else if(ty1->isfunction()) {
        if(!ty2->isfunction())
            goto err;
        if(ty1->fnarg.size() != ty2->fnarg.size())
            goto err;
        if(ty1->fnret->get().type != ty1->fnret->get().type)
            goto err;

        int i = ty1->fnarg.size();
        int cnt = 0;

        if(i == 0)
            return ty1;

        for(;;) {
            checktype(ty1->fnarg[cnt], ty2->fnarg[cnt]);
            ++cnt;
            if(cnt == i)
                return ty1;
        }
    }

    if(ty1->get().type == ty2->get().type)
        return ty1;
err:
    /*
    error(token.see(-1).line, token.see(-1).col,
            "expected type `%s`, found type `%s`",
            ty1->show().c_str(), ty2->show().c_str());*/ //TODO
    error("bad type: %s:%s", ty1->show().c_str(), ty2->show().c_str());

    return nullptr;
}
