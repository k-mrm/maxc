#include <string.h>

#include "sema.h"
#include "ast.h"
#include "error/error.h"
#include "struct.h"
#include "lexer.h"
#include "parser.h"
#include "builtins.h"
#include "namespace.h"
#include "module.h"

static Ast *visit(Ast *);
static Ast *visit_binary(Ast *);
static Ast *visit_unary(Ast *);
static Ast *visit_assign(Ast *);
static Ast *visit_dotexpr(Ast *);
static Ast *visit_subscr(Ast *);
static Ast *visit_object(Ast *);
static Ast *visit_struct_init(Ast *);
static Ast *visit_block(Ast *);
static Ast *visit_typed_block(Ast *);
static Ast *visit_list(Ast *);
static Ast *visit_if(Ast *);
static Ast *visit_for(Ast *);
static Ast *visit_exprif(Ast *);
static Ast *visit_while(Ast *);
static Ast *visit_return(Ast *);
static Ast *visit_vardecl(Ast *);
static Ast *visit_load(Ast *);
static Ast *visit_funcdef(Ast *);
static Ast *visit_fncall(Ast *);
static Ast *visit_fncall_impl(Ast *, Ast **, Vector *);
static Ast *visit_break(Ast *);
static Ast *visit_skip(Ast *);
static Ast *visit_bltinfn_call(Ast *, Ast **, Vector *);
static Ast *visit_namespace(Ast *);
static Ast *visit_namesolver(Ast *);
static Ast *visit_assert(Ast *);

static NodeVariable *determine_variable(char *, Scope);
static NodeVariable *determine_overload(NodeVariable *, Vector *);
static Type *solve_type(Type *);
static Type *checktype(Type *, Type *);
static Type *checktype_optional(Type *, Type *);

Scope scope;
FuncEnv fnenv;
Vector *fn_saver;
static int loop_nest = 0;

int ngvar = 0;

void sema_init() {
    scope.current = New_Env_Global();
    fnenv.current = New_Env_Global();
    fn_saver = New_Vector();

    setup_bltin();
}

SemaResult sema_analysis_repl(Vector *ast) {
    ast->data[0] = visit((Ast *)ast->data[0]);
    Ast *stmt = (Ast *)ast->data[0];

    var_set_number(fnenv.current->vars);

    scope_escape(&scope);

    bool isexpr = Ast_isexpr(stmt);
    char *typestr;
    if(isexpr && stmt && stmt->ctype) {
        typestr = stmt->ctype->tostring(stmt->ctype);
    }
    else {
        typestr = "";
    }

    return (SemaResult){ isexpr, typestr };
}

int sema_analysis(Vector *ast) {
    for(int i = 0; i < ast->len; ++i) {
        ast->data[i] = visit((Ast *)ast->data[i]);
    }

    ngvar += var_set_number(fnenv.current->vars);

    scope_escape(&scope);

    return ngvar;
}

void setup_bltin() {
    Varlist *bltfns = New_Varlist();
    for(int i = 0; i < Global_Cbltins->len; ++i) {
        NodeVariable *a =
            ((MxcCBltin *)Global_Cbltins->data[i])->var;
        a->isglobal = true;
        a->isbuiltin = true;
        a->is_overload = false;

        varlist_push(bltfns, a);
    }

    varlist_mulpush(fnenv.current->vars, bltfns);
    varlist_mulpush(scope.current->vars, bltfns);
}

static Ast *visit(Ast *ast) {
    if(!ast) return NULL;

    switch(ast->type) {
    case NDTYPE_NUM:
    case NDTYPE_BOOL:
    case NDTYPE_NULL:
    case NDTYPE_CHAR:
    case NDTYPE_STRING:
        break;
    case NDTYPE_LIST: return visit_list(ast);
    case NDTYPE_SUBSCR: return visit_subscr(ast);
    case NDTYPE_TUPLE:
        mxc_unimplemented("tuple");
        return ast;
    case NDTYPE_OBJECT: return visit_object(ast);
    case NDTYPE_STRUCTINIT: return visit_struct_init(ast);
    case NDTYPE_BINARY: return visit_binary(ast);
    case NDTYPE_DOTEXPR: return visit_dotexpr(ast);
    case NDTYPE_UNARY: return visit_unary(ast);
    case NDTYPE_ASSIGNMENT: return visit_assign(ast);
    case NDTYPE_IF: return visit_if(ast);
    case NDTYPE_EXPRIF: return visit_exprif(ast);
    case NDTYPE_FOR: return visit_for(ast);
    case NDTYPE_WHILE: return visit_while(ast);
    case NDTYPE_BLOCK: return visit_block(ast);
    case NDTYPE_TYPEDBLOCK: return visit_typed_block(ast);
    case NDTYPE_RETURN: return visit_return(ast);
    case NDTYPE_BREAK: return visit_break(ast);
    case NDTYPE_SKIP: return visit_skip(ast);
    case NDTYPE_BREAKPOINT: break;
    case NDTYPE_VARIABLE: return visit_load(ast);
    case NDTYPE_FUNCCALL: return visit_fncall(ast);
    case NDTYPE_FUNCDEF: return visit_funcdef(ast);
    case NDTYPE_VARDECL: return visit_vardecl(ast);
    case NDTYPE_NAMESPACE: return visit_namespace(ast);
    case NDTYPE_NAMESOLVER: return visit_namesolver(ast);
    case NDTYPE_ASSERT: return visit_assert(ast);
    case NDTYPE_NONENODE: break;
    default: mxc_assert(0, "unimplemented node");
    }

    return ast;
}

static Ast *visit_list_with_size(NodeList *l) {
    l->nelem = visit(l->nelem); 
    if(!l->nelem) return NULL;
    if(!checktype(l->nelem->ctype, mxcty_int)) {
        error("Number of elements in array must be numeric");
        return NULL;
    }

    l->init = visit(l->init);
    if(!l->init) return NULL;
    l->init->ctype = solve_type(l->init->ctype);
    CTYPE(l) = New_Type_With_Ptr(l->init->ctype);

    return CAST_AST(l);
}

static Ast *visit_list(Ast *ast) {
    NodeList *l = (NodeList *)ast;
    if(l->nelem) {
        return visit_list_with_size(l);
    }

    Type *base = NULL;

    if(l->nsize == 0) {
        // TODO
    }
    else {
        l->elem->data[0] = visit((Ast *)l->elem->data[0]);

        if(!l->elem->data[0]) return NULL;
        base = CTYPE(l->elem->data[0]);

        for(size_t i = 1; i < l->nsize; ++i) {
            Ast *el = (Ast *)l->elem->data[i];

            el = visit(el);

            if(!checktype(base, el->ctype)) {
                if(!base || !el->ctype)
                    return NULL;

                error("expect `%s`, found `%s`",
                      base->tostring(base),
                      el->ctype->tostring(el->ctype));
                return NULL;
            }
        }
    }

    CTYPE(l) = New_Type_With_Ptr(base);

    return (Ast *)l;
}

static Ast *visit_binary(Ast *ast) {
    NodeBinop *b = (NodeBinop *)ast;

    b->left = visit(b->left);
    b->right = visit(b->right);

    if(!b->left || !b->left->ctype) return NULL;
    if(!b->right || !b->right->ctype) return NULL;

    MxcOperator *res = chk_operator_type(b->left->ctype->defop,
                                         OPE_BINARY,
                                         b->op,
                                         b->right->ctype);

    if(!res) {
        error("undefined binary operation: `%s` %s `%s`",
                b->left->ctype->tostring(b->left->ctype),
                operator_dump(OPE_BINARY, b->op),
                b->right->ctype->tostring(b->right->ctype)
             );

        goto err;
    }

    CTYPE(b) = res->ret;

err:
    return CAST_AST(b);
}

static Ast *visit_unary(Ast *ast) {
    NodeUnaop *u = (NodeUnaop *)ast;

    u->expr = visit(u->expr);
    if(!u->expr || !u->expr->ctype) return NULL;

    MxcOperator *res = chk_operator_type(u->expr->ctype->defop,
                                         OPE_UNARY,
                                         u->op,
                                         NULL);

    if(!res) {
        error("undefined unary operation `%s` to `%s`",
              operator_dump(OPE_UNARY, u->op),
              u->expr->ctype->tostring(u->expr->ctype));
        return NULL;
    }

    if(res->op == UNA_INC || res->op == UNA_DEC) {
        switch(u->expr->type) {
        case NDTYPE_VARIABLE:
        case NDTYPE_SUBSCR:
        case NDTYPE_MEMBER:
            break;
        default:
            error("`%s` cannot be applied to literal", res->opname);
            return NULL;
        }
    }
    CTYPE(u) = res->ret;

    return CAST_AST(u);
}

static Ast *visit_var_assign(NodeAssignment *a) {
    NodeVariable *v = (NodeVariable *)a->dst;

    if(v->vattr & VARATTR_CONST) {
        error("assignment of read-only variable: %s", v->name);
        return NULL;
    }
    v->vattr &= ~(VARATTR_UNINIT);

    if(!checktype(a->dst->ctype, a->src->ctype)) {
        if(!a->dst->ctype || !a->src->ctype) return NULL;

        error("type error `%s`, `%s`",
              a->dst->ctype->tostring(a->dst->ctype),
              a->src->ctype->tostring(a->src->ctype));
        return NULL;
    }

    CTYPE(a)= a->dst->ctype;

    return CAST_AST(a);
}

static Ast *visit_subscr_assign(NodeAssignment *a) {
    if(!checktype(a->dst->ctype, a->src->ctype)) {
        if(!a->dst->ctype || !a->src->ctype)
            return NULL;

        error("type error `%s`, `%s`",
              a->dst->ctype->tostring(a->dst->ctype),
              a->src->ctype->tostring(a->src->ctype));
        return NULL;
    }

    CTYPE(a)= a->dst->ctype;

    return CAST_AST(a);
}

static Ast *visit_member_assign(NodeAssignment *a) {
    if(!checktype(a->dst->ctype, a->src->ctype)) {
        if(!a->dst->ctype || !a->src->ctype)
            return NULL;

        error("type error `%s`, `%s`",
              a->dst->ctype->tostring(a->dst->ctype),
              a->src->ctype->tostring(a->src->ctype));
        return NULL;
    }

    CTYPE(a)= a->dst->ctype;

    return CAST_AST(a);
}

static Ast *visit_assign(Ast *ast) {
    NodeAssignment *a = (NodeAssignment *)ast;
    a->dst = visit(a->dst);
    a->src = visit(a->src);
    if(!a->dst || !a->src) return NULL;

    switch(a->dst->type) {
    case NDTYPE_VARIABLE:   return visit_var_assign(a);
    case NDTYPE_SUBSCR:     return visit_subscr_assign(a);
    case NDTYPE_DOTEXPR:
        if(((NodeDotExpr *)a->dst)->t.member)
            return visit_member_assign(a);
        /* fall through */
    default:
        error("left side of the expression is not valid");

        return NULL;
    }
}

static Ast *visit_subscr(Ast *ast) {
    NodeSubscript *s = (NodeSubscript *)ast;

    s->ls = visit(s->ls);
    s->index = visit(s->index);

    if(!s->ls) return NULL;
    if(!CTYPE(s->ls)) return NULL;

    if(!CTYPE(s->ls)->ptr) {
        error("cannot index into a value of type `%s`",
              s->ls->ctype->tostring(s->ls->ctype));
        return NULL;
    }
    CTYPE(s)= s->ls->ctype->ptr;

    return (Ast *)s;
}

static Ast *visit_member_impl(Ast *self, Ast **left, Ast **right) {
    if(!*left || !(*left)->ctype) return NULL;

    if(!is_struct((*left)->ctype)) {
        return NULL;
    }

    if((*right)->type == NDTYPE_VARIABLE) {
        NodeVariable *rhs = (NodeVariable *)*right;
        size_t nfield = (*left)->ctype->strct.nfield;

        for(size_t i = 0; i < nfield; ++i) {
            if(strncmp((*left)->ctype->strct.field[i]->name,
                       rhs->name,
                       strlen((*left)->ctype->strct.field[i]->name)) == 0) {
                Type *fieldty = CTYPE((*left)->ctype->strct.field[i]);
                self->ctype = solve_type(fieldty);
                return self;
            }
        }
    }

    return NULL;
}

static Ast *visit_dotexpr(Ast *ast) {
    NodeDotExpr *d = (NodeDotExpr *)ast;
    d->left = visit(d->left);
    if(!d->left) return NULL;
    NodeDotExpr *res;

    res = (NodeDotExpr *)visit_member_impl((Ast *)d, &d->left, &d->right);
    if(res) {
        d = res;
        d->t.member = 1;
        d->memb = new_node_member(d->left, d->right);
        CTYPE(d->memb)= CTYPE(res);
        return CAST_AST(d);
    }

    d->right = visit(d->right);
    Vector *arg = New_Vector_With_Size(1);
    arg->data[0] = d->left;
    res = (NodeDotExpr *)visit_fncall_impl((Ast *)d, &d->right, arg);
    if(res) {
        d = res;
        d->t.fncall = 1;
        d->call = new_node_fncall(d->right, arg, NULL);
        CTYPE(d->call)= CTYPE(res);
        return CAST_AST(d);
    }

    return NULL;
}

static Ast *visit_object(Ast *ast) {
    NodeObject *s = (NodeObject *)ast;

    mxc_assert(CAST_AST(s->decls->data[0])->type == NDTYPE_VARIABLE,
               "internal error");

    MxcStruct struct_info = New_MxcStruct(
        s->tagname, (NodeVariable **)s->decls->data, s->decls->len);

    vec_push(scope.current->userdef_type, New_Type_With_Struct(struct_info));

    return CAST_AST(s);
}

static Ast *visit_struct_init(Ast *ast) {
    NodeStructInit *s = (NodeStructInit *)ast;

    s->tag = solve_type(s->tag);
    CTYPE(s)= s->tag;

    for(int i = 0; i < s->fields->len; ++i) {
        // TODO
        ;
    }
    for(int i = 0; i < s->inits->len; ++i) {
        s->inits->data[i] = visit(s->inits->data[i]);
    }

    return CAST_AST(s);
}

static Ast *visit_block(Ast *ast) {
    NodeBlock *b = (NodeBlock *)ast;
    scope_make(&scope);

    for(int i = 0; i < b->cont->len; ++i) {
        b->cont->data[i] = visit(b->cont->data[i]);
    }

    scope_escape(&scope);

    return CAST_AST(b);
}

static Ast *visit_typed_block(Ast *ast) {
    NodeBlock *b = (NodeBlock *)ast;
    scope_make(&scope);

    for(int i = 0; i < b->cont->len; ++i) {
        b->cont->data[i] = visit(b->cont->data[i]);
    }

    scope_escape(&scope);
    CTYPE(b)= CTYPE(b->cont->data[b->cont->len - 1]);

    return CAST_AST(b);
}

static Ast *visit_if(Ast *ast) {
    NodeIf *i = (NodeIf *)ast;
    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    CTYPE(i)= mxcty_none;

    return CAST_AST(i);
}

static Ast *visit_exprif(Ast *ast) {
    NodeIf *i = (NodeIf *)ast;
    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    if(!i->then_s || !i->else_s) return NULL;

    CTYPE(i)= checktype(i->then_s->ctype, i->else_s->ctype);

    return CAST_AST(i);
}

static Ast *visit_for(Ast *ast) {
    NodeFor *f = (NodeFor *)ast;
    f->iter = visit(f->iter);
    if(!f->iter) return NULL;

    if(!is_iterable(f->iter->ctype)) {
        if(!f->iter->ctype) return NULL;

        error("%s is not an iterable object",
              f->iter->ctype->tostring(f->iter->ctype));
        return NULL;
    }

    bool isglobal = funcenv_isglobal(fnenv);

    scope_make(&scope);

    for(int i = 0; i < f->vars->len; i++) {
        CTYPE(f->vars->data[i])= f->iter->ctype->ptr;
        ((NodeVariable *)f->vars->data[i])->isglobal = isglobal;

        varlist_push(fnenv.current->vars, f->vars->data[i]);
        varlist_push(scope.current->vars, f->vars->data[i]);

        f->vars->data[i] = visit(f->vars->data[i]);
    }

    f->body = visit(f->body);

    scope_escape(&scope);

    return CAST_AST(f);
}

static Ast *visit_while(Ast *ast) {
    NodeWhile *w = (NodeWhile *)ast;
    w->cond = visit(w->cond);

    loop_nest++;
    w->body = visit(w->body);
    loop_nest--;

    return CAST_AST(w);
}

static Ast *visit_return(Ast *ast) {
    NodeReturn *r = (NodeReturn *)ast;
    r->cont = visit(r->cont);
    if(!r->cont) return NULL;

    if(fn_saver->len == 0) {
        error("use of return statement outside function or block");
        return NULL;
    }

    Type *cur_fn_retty =
        CTYPE(((NodeFunction *)vec_last(fn_saver))->fnvar)->fnret;

    if(!checktype(cur_fn_retty, r->cont->ctype)) {
        if(type_is(cur_fn_retty, CTYPE_OPTIONAL)) {
            if(!type_is(r->cont->ctype, CTYPE_ERROR)) {
                if(!r->cont->ctype) return NULL;

                error("return type error: expected error, found %s",
                        r->cont->ctype->tostring(r->cont->ctype));
                return NULL;
            }
        }
        else {
            if(!cur_fn_retty || !r->cont->ctype) return NULL;

            error("type error: expected %s, found %s",
                    cur_fn_retty->tostring(cur_fn_retty),
                    r->cont->ctype->tostring(r->cont->ctype));
        }
    }

    return CAST_AST(r);
}

static Ast *visit_break(Ast *ast) {
    NodeBreak *b = (NodeBreak *)ast;
    if(loop_nest == 0) {
        error("break statement must be inside loop statement");
        return NULL;
    }

    return (Ast *)b;
}

static Ast *visit_skip(Ast *ast) {
    NodeSkip *s = (NodeSkip *)ast;
    if(loop_nest == 0) {
        error("skip statement must be inside loop statement");
        return NULL;
    }

    return (Ast *)s;
}

static Ast *visit_vardecl_block(NodeVardecl *v) {
    for(int i = 0; i < v->block->len; ++i) {
        v->block->data[i] = visit_vardecl(v->block->data[i]);
    }

    return (Ast *)v;
}

static Ast *visit_vardecl(Ast *ast) {
    NodeVardecl *v = (NodeVardecl *)ast;
    if(v->is_block) {
        return visit_vardecl_block(v);
    }

    v->var->isglobal = funcenv_isglobal(fnenv);

    if(v->init) {
        v->init = visit(v->init);
        if(!v->init) return NULL;

        if(type_is(CTYPE(v->var), CTYPE_UNINFERRED)) {
            CTYPE(v->var) = v->init->ctype;
        }
        else if(!checktype(CTYPE(v->var), v->init->ctype)) {
            if(!CTYPE(v->var)) return NULL;

            error( "`%s` type is %s",
                    v->var->name,
                    CTYPE(v->var)->tostring(CTYPE(v->var)));
            return NULL;
        }
    }
    else {
        if(type_is(CAST_AST(v->var)->ctype, CTYPE_UNINFERRED)) {
            error("Must always be initialized when doing type inference.");
            return NULL;
        }

        CAST_AST(v->var)->ctype = solve_type(CAST_AST(v->var)->ctype);

        v->var->vattr |= VARATTR_UNINIT;
    }

    varlist_push(fnenv.current->vars, v->var);
    if(chk_var_conflict(scope, v->var)) {
        error("conflict var-declaration `%s`", v->var->name);
        return NULL;
    }
    varlist_push(scope.current->vars, v->var);

    return CAST_AST(v);
}

static Ast *visit_fncall_impl(Ast *self, Ast **ast, Vector *arg) {
    Vector *argtys = New_Vector();
    for(int i = 0; i < arg->len; ++i) {
        if(arg->data[i])
            vec_push(argtys, CAST_AST(arg->data[i])->ctype);
    }

    if(!*ast) return NULL;

    if(!type_is(CTYPE(*ast), CTYPE_FUNCTION)) {
        if(!CTYPE(*ast)) return NULL;

        error("`%s` is not function object",
              CTYPE(*ast)->tostring(CTYPE(*ast)));
        return NULL;
    }

    if((*ast)->type == NDTYPE_VARIABLE) {
        *ast = (Ast *)determine_overload((NodeVariable *)*ast,
                argtys);
    }
    else {
        mxc_unimplemented("error");
        // TODO
    }

    if(!*ast) return NULL;

    NodeVariable *fn = (NodeVariable *)*ast;
    if(fn->isbuiltin) {
        return visit_bltinfn_call(self, ast, argtys);
    }
    self->ctype = CTYPE(fn)->fnret;

    return self;
}

static Ast *visit_fncall(Ast *ast) {
    NodeFnCall *f = (NodeFnCall *)ast;
    f->func = visit(f->func);
    for(size_t i = 0; i < f->args->len; ++i) {
        f->args->data[i] = visit((Ast *)f->args->data[i]);
    }
    f = (NodeFnCall *)visit_fncall_impl((Ast *)f, &f->func, f->args);

    return (Ast *)f;
}

static Ast *visit_funcdef(Ast *ast) {
    NodeFunction *fn = (NodeFunction *)ast;

    vec_push(fn_saver, fn);
    fn->fnvar->isglobal = funcenv_isglobal(fnenv);
    NodeVariable *registerd_var = determine_variable(fn->fnvar->name, scope);

    if(!registerd_var) {
        /* not registerd in scope */
        varlist_push(fnenv.current->vars, fn->fnvar);
        varlist_push(scope.current->vars, fn->fnvar);
        fn->fnvar->is_overload = false;
        fn->fnvar->next = NULL;
    }
    else {
        registerd_var->is_overload = true;
        while(registerd_var->next) {
            registerd_var = registerd_var->next;
        }
        registerd_var->next = fn->fnvar;
    }

    funcenv_make(&fnenv);
    scope_make(&scope);

    fn->fnvar->vattr = 0;
    if(fn->is_generic) {
        for(int i = 0; i < fn->typevars->len; i++) {
            vec_push(scope.current->userdef_type,
                     (Type *)fn->typevars->data[i]);
        }
    }

    /* register arguments in the environment */
    for(int i = 0; i < fn->args->vars->len; ++i) {
        NodeVariable *cur = (NodeVariable *)fn->args->vars->data[i];
        cur->isglobal = false;

        CTYPE(fn->fnvar)->fnarg->data[i] =
                solve_type(CTYPE(fn->fnvar)->fnarg->data[i]);

        varlist_push(fnenv.current->vars, cur);
        varlist_push(scope.current->vars, cur);
    }

    fn->block = visit(fn->block);
    if(!fn->block) return NULL;

    if(fn->block->type != NDTYPE_BLOCK) {
        // expr
        if(type_is(CTYPE(fn->fnvar)->fnret, CTYPE_UNINFERRED)) {
            CTYPE(fn->fnvar)->fnret = fn->block->ctype;
        }
        else {
            CTYPE(fn->fnvar)->fnret = solve_type(CTYPE(fn->fnvar)->fnret);
            Type *suc = checktype(CTYPE(fn->fnvar)->fnret, fn->block->ctype);
            if(!suc) {
                error("return type error");
                return NULL;
            }
        }
    }
    var_set_number(fnenv.current->vars);
    fn->lvars = fnenv.current->vars;

    funcenv_escape(&fnenv);
    scope_escape(&scope);

    vec_pop(fn_saver);

    return CAST_AST(fn);
}

static bool print_arg_check(Vector *argtys) {
    for(int i = 0; i < argtys->len; i++) {
        if(!argtys->data[i]) {}
        else if(!(((Type *)argtys->data[i])->impl & TIMPL_SHOW)) {
            if(!argtys->data[i]) return NULL;

            error("type %s does not implement `Show`",
                    ((Type *)argtys->data[i])->tostring((Type *)argtys->data[i]));

            return false;
        }
    }

    return true;
}

static Ast *visit_bltinfn_call(Ast *self, Ast **func, Vector *argtys) {
    NodeVariable *fn = (NodeVariable *)*func;

    if(!fn) return NULL;

    if(strcmp(fn->name, "print") == 0 ||
       strcmp(fn->name, "println") == 0) {
        print_arg_check(argtys);
    }

    self->ctype = CTYPE(fn)->fnret;

    return self;
}

static Ast *visit_load(Ast *ast) {
    NodeVariable *v = (NodeVariable *)ast;
    NodeVariable *res = determine_variable(v->name, scope);
    if(!res) {
        error("undeclared variable: %s", v->name);
        return NULL;
    }
    v = res;

    CTYPE(v) = solve_type(CTYPE(v));
    if((v->vattr & VARATTR_UNINIT) &&
       !type_is(CAST_AST(v)->ctype, CTYPE_STRUCT)) {
        error("use of uninit variable: %s", v->name);
        return NULL;
    }

    v->used = true;

    return CAST_AST(v);
}

static Ast *visit_namespace(Ast *ast) {
    NodeNameSpace *s = (NodeNameSpace *)ast;
    scope_make(&scope);

    for(int i = 0; i < s->block->cont->len; ++i) {
        s->block->cont->data[i] = visit(s->block->cont->data[i]);
    }

    Register_Namespace(s->name, scope.current->vars);

    scope_escape(&scope);

    return CAST_AST(s);
}

static Ast *visit_assert(Ast *ast) {
    NodeAssert *a = (NodeAssert *)ast;
    a->cond = visit(a->cond);
    if(!a->cond) return NULL;

    if(!checktype(a->cond->ctype, mxcty_bool)) {
        error("assert conditional expression type must be"
                "`bool`, but got %s",
                a->cond->ctype->tostring(a->cond->ctype));

        return NULL;
    }

    return (Ast *)a;
}

static Ast *visit_namesolver(Ast *ast) {
    NodeNameSolver *v = (NodeNameSolver *)ast;
    NodeVariable *ns_name = (NodeVariable *)v->name;
    Varlist *vars = Search_Namespace(ns_name->name);
    if(!vars) {
        error("unknown namespace: `%s`", ns_name->name);
        return NULL;
    }
    NodeVariable *id = (NodeVariable *)v->ident;

    for(size_t i = 0; i < vars->vars->len; ++i) {
        NodeVariable *cur = (NodeVariable *)vars->vars->data[i];

        if(strcmp(id->name, cur->name) == 0) {
            return (Ast *)cur;
        }
    }

    error("undeclared variable: `%s` in %s", id->name, ns_name->name);
    return NULL;
}

static NodeVariable *determine_variable(char *name, Scope scope) {
    for(Env *e = scope.current;; e = e->parent) {
        if(!e->vars->vars->len == 0)
            break;
        if(e->isglb) {
            // debug("empty\n");
            goto verr;
        }
    }

    for(Env *e = scope.current;; e = e->parent) {
        for(int i = 0; i < e->vars->vars->len; ++i) {
            if(strcmp(((NodeVariable *)e->vars->vars->data[i])->name,
                      name) == 0) {
                return (NodeVariable *)e->vars->vars->data[i];
            }
        }
        if(e->isglb) {
            // debug("it is glooobal\n");
            goto verr;
        }
    }

verr:
    return NULL;
}

static NodeVariable *determine_overload(NodeVariable *var,
                                        Vector *argtys) {
    char *fname = var ? var->name : "";
    do {
        if(!var) return NULL;
        if(CTYPE(var)->fnarg->len == 0) {
            if(argtys->len == 0)
                return var;
            else
                continue;
        }

        if(CAST_TYPE(CTYPE(var)->fnarg->data[0])->type ==
                CTYPE_ANY_VARARG) {
            return var;
        }

        if(CTYPE(var)->fnarg->len != argtys->len) {
            continue;
        }

        if(CAST_TYPE(CTYPE(var)->fnarg->data[0])->type ==
                CTYPE_ANY) {
            return var;
        }

        bool is_same = true;
        for(int i = 0; i < CTYPE(var)->fnarg->len; ++i) {
            if(!checktype(CAST_TYPE(CTYPE(var)->fnarg->data[i]),
                        CAST_TYPE(argtys->data[i]))) {
                is_same = false;
                break;
            }
        }

        if(is_same) return var;
    } while((var = var->next));

    error("Function not found: %s()", fname);
    return NULL;
}

static Type *checktype_optional(Type *ty1, Type *ty2) {
    Type *checked = checktype(ty1, ty2);

    if(checked) {
        return checked;
    }

    if(ty1->optional) {
        if(ty2->type == CTYPE_ERROR) {
            return ty1;
        }
    }

    return NULL;
}

static Type *checktype(Type *ty1, Type *ty2) {
    if(!ty1 || !ty2) return NULL;

    ty1 = instantiate(ty1);
    ty2 = instantiate(ty2);

    ty1 = solve_type(ty1);
    ty2 = solve_type(ty2);

    if(type_is(ty1, CTYPE_OPTIONAL)) {
        ty1 = ((MxcOptional *)ty1)->base;
    }
    if(type_is(ty2, CTYPE_OPTIONAL)) {
        ty2 = ((MxcOptional *)ty2)->base;
    }

    if(type_is(ty1, CTYPE_LIST)) {
        if(!type_is(ty2, CTYPE_LIST))
            goto err;

        Type *a = ty1;
        Type *b = ty2;

        for(;;) {
            a = a->ptr;
            b = b->ptr;

            if(!a && !b)
                return ty1;
            if(!a || !b)
                goto err;
            if(!checktype(a, b)) {
                goto err;
            }
        }
    }
    else if(ty1->type == CTYPE_STRUCT &&
            ty2->type == CTYPE_STRUCT) {
        if(strcmp(ty1->strct.name, ty2->strct.name) == 0) {
            return ty1;
        }
        else {
            goto err;
        }
    }
    else if(type_is(ty1, CTYPE_TUPLE)) {
        if(!type_is(ty2, CTYPE_TUPLE))
            goto err;
        if(ty1->tuple->len != ty2->tuple->len)
            goto err;
        int s = ty1->tuple->len;
        int cnt = 0;

        for(;;) {
            checktype(ty1->tuple->data[cnt], ty2->tuple->data[cnt]);
            ++cnt;
            if(cnt == s)
                return ty1;
        }
    }
    else if(type_is(ty1, CTYPE_FUNCTION)) {
        if(!type_is(ty2, CTYPE_FUNCTION))
            goto err;
        if(ty1->fnarg->len != ty2->fnarg->len)
            goto err;
        if(ty1->fnret->type != ty2->fnret->type)
            goto err;

        int i = ty1->fnarg->len;
        int cnt = 0;

        if(i == 0)
            return ty1;

        for(;;) {
            if(!checktype(ty1->fnarg->data[cnt], ty2->fnarg->data[cnt])) {
                if(!ty1->fnarg->data[cnt] || !ty2->fnarg->data[cnt]) {
                    return NULL;
                }
                Type *err1 = (Type *)ty1->fnarg->data[cnt];
                Type *err2 = (Type *)ty2->fnarg->data[cnt];

                error("type error `%s`, `%s`",
                      err1->tostring(err1),
                      err2->tostring(err2));
            }
            ++cnt;
            if(cnt == i)
                return ty1;
        }
    }

    //primitive
    if(ty1->type == ty2->type)
        return ty1;

err:
    return NULL;
}

static Type *solve_type(Type *ty) {
    if(!is_unsolved(ty)) return ty;

    for(Env *e = scope.current;; e = e->parent) {
        if(!e->userdef_type->len == 0)
            break;
        if(e->isglb) {
            //debug("empty\n");
            goto err;
        }
    }

    for(Env *e = scope.current;; e = e->parent) {
        for(int i = 0; i < e->userdef_type->len; ++i) {
            Type *ud = (Type *)e->userdef_type->data[i];

            if(type_is(ud, CTYPE_STRUCT)) {
                if(strcmp(ud->strct.name, ty->name) == 0) {
                    return ud;
                }
            }
            else if(type_is(ud, CTYPE_VARIABLE)) {
                if(strcmp(ud->type_name, ty->name) == 0) {
                    return ud;
                }
            }
        }
        if(e->isglb) {
            goto err;
        }
    }

err:
    error("undefined type: %s", ty->name);

    return ty;
}
