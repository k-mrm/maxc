#include "sema.h"
#include "error.h"
#include "maxc.h"

Ast_v &SemaAnalyzer::run(Ast_v &ast) {
    scope.current = new env_t(true);
    fnenv.current = new env_t(true);

    setup_bltin();

    for(Ast *a : ast)
        ret_ast.push_back(visit(a));

    ngvar = fnenv.current->vars.get().size();

    fnenv.current->vars.set_number();

    return ret_ast;
}

void SemaAnalyzer::setup_bltin() {
    std::vector<std::string> bltfns_name = {
        "print",
        "println",
        "objectid",
        "len",
    };
    std::vector<BltinFnKind> bltfns_kind = {
        BltinFnKind::Print,
        BltinFnKind::Println,
        BltinFnKind::ObjectId,
        BltinFnKind::StringSize,
    };

    std::vector<NodeVariable *> bltfns;

    for(size_t i = 0; i < bltfns_name.size(); ++i) {
        Type *fntype = new Type(CTYPE::FUNCTION);

        fntype = set_bltinfn_type(bltfns_kind[i], fntype);

        func_t finfo = func_t(bltfns_kind[i], fntype);

        NodeVariable *a = new NodeVariable(bltfns_name[i], finfo);
        a->isglobal = true;

        bltfns.push_back(a);
    }

    fnenv.current->vars.push(bltfns);
    scope.current->vars.push(bltfns);
}

Type *SemaAnalyzer::set_bltinfn_type(BltinFnKind kind, Type *ty) {
    switch(kind) {
    case BltinFnKind::Print:
    case BltinFnKind::Println:
        ty->fnret = mxcty_none;
        ty->fnarg = {new Type(CTYPE::ANY_VARARG)};
        break;
    case BltinFnKind::ObjectId:
        ty->fnret = mxcty_int;
        ty->fnarg = {new Type(CTYPE::ANY)};
        break;
    case BltinFnKind::StringSize:
        ty->fnret = mxcty_int;
        ty->fnarg = {new Type(CTYPE::STRING)};
        break;
    default:
        error("maxc internal error");
    }

    return ty;
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
    case NDTYPE::MEMBER:
        return visit_member(ast);
    case NDTYPE::UNARY:
        return visit_unary(ast);
    case NDTYPE::TERNARY:
        break;
    case NDTYPE::ASSIGNMENT:
        return visit_assign(ast);
    case NDTYPE::IF:
        return visit_if(ast);
    case NDTYPE::EXPRIF:
        return visit_exprif(ast);
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

    if(b->op == "<" || b->op == ">" || b->op == "<=" || b->op == ">=" ||
       b->op == "!=" || b->op == "==") {
        checktype(b->left->ctype, b->right->ctype);

        b->ctype = mxcty_bool;
    }
    else {
        b->ctype = checktype(b->left->ctype, b->right->ctype);
        b->left->ctype = b->ctype;
        b->right->ctype = b->ctype;
    }

    return b;
}

Ast *SemaAnalyzer::visit_unary(Ast *ast) {
    auto u = (NodeUnaop *)ast;

    u->expr = visit(u->expr);

    u->ctype = u->expr->ctype;

    return u;
}

Ast *SemaAnalyzer::visit_assign(Ast *ast) {
    auto a = (NodeAssignment *)ast;

    if(a->dst->get_nd_type() != NDTYPE::VARIABLE &&
       a->dst->get_nd_type() != NDTYPE::SUBSCR) {
        error("left side of the expression is not valid");
    }

    a->dst = (NodeVariable *)visit(a->dst);

    NodeVariable *v = (NodeVariable *)a->dst;
    // TODO: subscr?

    if(v->vinfo.vattr & (int)VarAttr::Const) {
        error("assignment of read-only variable: %s", v->name);
    }

    a->src = visit(a->src);

    v->vinfo.vattr &= ~((int)VarAttr::Uninit);

    checktype(a->dst->ctype, a->src->ctype);

    return a;
}

Ast *SemaAnalyzer::visit_member(Ast *ast) {
    auto m = (NodeMember *)ast;

    m->left = visit(m->left);

    if(m->right->get_nd_type() == NDTYPE::VARIABLE) {
        // field
    }
    else {
        error("unimplemented!: member");
    }

    return m;
}

Ast *SemaAnalyzer::visit_block(Ast *ast) {
    auto b = (NodeBlock *)ast;

    scope.make();

    for(auto &a : b->cont) {
        a = visit(a);
    }

    scope.escape();

    return b;
}

Ast *SemaAnalyzer::visit_if(Ast *ast) {
    auto i = (NodeIf *)ast;

    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    i->ctype = nullptr;

    return i;
}

Ast *SemaAnalyzer::visit_exprif(Ast *ast) {
    auto i = (NodeIf *)ast;

    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    i->ctype = checktype(i->then_s->ctype, i->else_s->ctype);

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

    v->var->isglobal = fnenv.isglobal();

    if(v->init != nullptr) {
        v->init = visit(v->init);

        if(v->var->ctype->uninfer()) {
            v->var->ctype = v->init->ctype;
        }
        else if(!checktype(v->var->ctype, v->init->ctype)) {
            error("%s type is %s", v->var->name, v->var->ctype->show().c_str());
        }
    }
    else {
        if(v->var->ctype->uninfer()) {
            error("Must always be initialized when doing type inference.");
        }

        v->var->vinfo.vattr |= (int)VarAttr::Uninit;
    }

    fnenv.current->vars.push(v->var);
    scope.current->vars.push(v->var);

    return v;
}

Ast *SemaAnalyzer::visit_fncall(Ast *ast) {
    auto f = (NodeFnCall *)ast;

    Type_v argtys = {};

    for(size_t i = 0; i < f->args.size(); ++i) {
        f->args[i] = visit(f->args[i]);
        argtys.push_back(f->args[i]->ctype);
    }

    f->func = visit(f->func);

    if(f->func->get_nd_type() == NDTYPE::VARIABLE)
        f->func = determining_overload((NodeVariable *)f->func, argtys);

    if(((NodeVariable *)f->func)->finfo.isbuiltin) {
        return visit_bltinfn_call(f);
    }

    NodeVariable *fn = (NodeVariable *)f->func;

    f->ctype = fn->finfo.ftype->fnret;

    return f;
}

Ast *SemaAnalyzer::visit_funcdef(Ast *ast) {
    NodeFunction *fn = (NodeFunction *)ast;

    fn_saver.push(fn);

    fn->fnvar->isglobal = fnenv.isglobal();

    fnenv.current->vars.push(fn->fnvar);
    scope.current->vars.push(fn->fnvar);

    fnenv.make();
    scope.make();

    fn->fnvar->vinfo.vattr = 0;

    for(auto &a : fn->finfo.args.get()) {
        a->isglobal = false;

        fnenv.current->vars.push(a);
        scope.current->vars.push(a);
    }

    fn->block = (NodeBlock *)visit(fn->block);

    fnenv.current->vars.set_number();

    fn->lvars = fnenv.current->vars;

    fnenv.escape();
    scope.escape();

    fn_saver.pop();

    return fn;
}

Ast *SemaAnalyzer::visit_bltinfn_call(NodeFnCall *f) {
    f->func = visit(f->func);

    NodeVariable *fn = (NodeVariable *)f->func;

    if(fn == nullptr) return nullptr;

    f->ctype = fn->ctype->fnret;

    return f;
}

Ast *SemaAnalyzer::visit_load(Ast *ast) {
    auto v = (NodeVariable *)ast;

    v = do_variable_determining(v->name);

    if(v->vinfo.vattr & (int)VarAttr::Uninit) {
        error("use of uninit variable: %s", v->name.c_str());
    }

    return v;
}

NodeVariable *SemaAnalyzer::do_variable_determining(std::string &name) {
    for(env_t *e = scope.current;; e = e->parent) {
        if(!e->vars.get().empty())
            break;
        if(e->isglb) {
            // debug("empty\n");
            goto verr;
        }
    }

    // fnenv.current->vars.show();
    for(env_t *e = scope.current;; e = e->parent) {
        for(auto &v : e->vars.get()) {
            if(v->name == name) {
                return v;
            }
        }
        if(e->isglb) {
            // debug("it is glooobal\n");
            goto verr;
        }
    }

verr:
    error("undeclared variable: %s", name.c_str());
    /*
    error(token.see(-1).line, token.see(-1).col,
            "undeclared variable: `%s`", tk.value.c_str());
    error(tk.start, tk.end, "undeclared variable: `%s`", tk.value.c_str());
    */
    return nullptr;
}

NodeVariable *SemaAnalyzer::determining_overload(NodeVariable *var,
                                                 Type_v &argtys) {
    if(var == nullptr) {
        return nullptr;
    }

    for(env_t *e = scope.current;; e = e->parent) {
        for(auto &v : e->vars.get()) {
            if(v->name == var->name) {
                debug("fnarg0:%s\n", v->ctype->fnarg[0]->show().c_str());
                // args size check
                if(v->ctype->fnarg[0]->get().type == CTYPE::ANY_VARARG)
                    return v;
                else if(v->ctype->fnarg[0]->get().type == CTYPE::ANY) {
                    if(argtys.size() == 1) return v;
                    else {
                        error("the number of %s() argument must be 1", v->name.c_str());
                        return nullptr;
                    }
                }

                if(v->ctype->fnarg.size() == argtys.size()) {
                    if(argtys.size() == 0)
                        return v;
                    // type check
                    for(size_t i = 0; i < v->ctype->fnarg.size(); ++i) {
                        if(v->ctype->fnarg[i]->get().type != argtys[i]->get().type)
                            break;
                        return v;
                    }
                }
            }
        }

        if(e->isglb) {
            debug("it is glooobal\n");
            goto err;
        }
    }

err:
    error("No Function!: %s", var->name.c_str());

    return nullptr;
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
