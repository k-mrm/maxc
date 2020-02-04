#include "sema.h"
#include "error/error.h"
#include "maxc.h"
#include "struct.h"
#include "lexer.h"
#include "parser.h"
#include "builtins.h"
#include "namespace.h"

static Ast *visit(Ast *);
static Type *set_bltinfn_type(enum BLTINFN);
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
    char *bltfns_name[] = {
        "print",
        "println",
        "objectid",
        "len",
        "tofloat",
        /* "add", */
        "error",
        "exit",
        "readline",
        "len",
    };
    enum BLTINFN bltfns_kind[] = {
        BLTINFN_PRINT,
        BLTINFN_PRINTLN,
        BLTINFN_OBJECTID,
        BLTINFN_STRINGSIZE,
        BLTINFN_INTTOFLOAT,
        /* BLTINFN_LISTADD, */
        BLTINFN_ERROR,
        BLTINFN_EXIT,
        BLTINFN_READLINE,
        BLTINFN_LISTLEN,
    };

    int nfn = sizeof(bltfns_kind) / sizeof(bltfns_kind[0]);

    Varlist *bltfns = New_Varlist();

    for(int i = 0; i < nfn; ++i) {
        Type *fntype = set_bltinfn_type(bltfns_kind[i]);

        func_t finfo = New_Func_t_With_Bltin(bltfns_kind[i], fntype, false);

        NodeVariable *a = new_node_variable_with_func(bltfns_name[i], finfo);
        a->isglobal = true;
        a->isbuiltin = true;
        a->is_overload = false;
        a->children = New_Vector();
        vec_push(a->children, a);

        varlist_push(bltfns, a);
    }

    varlist_mulpush(fnenv.current->vars, bltfns);
    varlist_mulpush(scope.current->vars, bltfns);

    bltin_funcs = bltfns;
}

static Type *set_bltinfn_type(enum BLTINFN kind) {
    Vector *fnarg = New_Vector();
    Type *fnret = mxcty_none;

    switch(kind) {
    case BLTINFN_PRINT:
    case BLTINFN_PRINTLN:
        vec_push(fnarg, mxcty_any_vararg);
        break;
    case BLTINFN_OBJECTID:
        fnret = mxcty_int;
        vec_push(fnarg, mxcty_any);
        break;
    case BLTINFN_STRINGSIZE:
        fnret = mxcty_int;
        vec_push(fnarg, mxcty_string);
        break;
    case BLTINFN_INTTOFLOAT:
        fnret = mxcty_float;
        vec_push(fnarg, mxcty_int);
        break;
    /*
    case BLTINFN_LISTADD: {
        Type *var = New_Type_Variable();

        ty->fnret = mxcty_none;
        vec_push(ty->fnarg, New_Type_With_Ptr(var));
        vec_push(ty->fnarg, var);
        break;
    }*/
    case BLTINFN_ERROR:
        fnret = New_Type(CTYPE_ERROR);
        vec_push(fnarg, mxcty_string);
        break;
    case BLTINFN_EXIT:
        vec_push(fnarg, mxcty_int);
        break;
    case BLTINFN_READLINE:
        fnret = mxcty_string;
        break;
    case BLTINFN_LISTLEN:
        // vec_push(fnarg, mxcty_anylist);
        fnret = mxcty_int;
        break;
    default:
        mxc_assert(0, "maxc internal error");
    }

    return New_Type_Function(fnarg, fnret);
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
    case NDTYPE_NONENODE: break;
    default: mxc_assert(0, "internal error");
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
    CAST_AST(l)->ctype = New_Type_With_Ptr(l->init->ctype);

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
        base = CAST_AST(l->elem->data[0])->ctype;

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

    CAST_AST(l)->ctype = New_Type_With_Ptr(base);

    return (Ast *)l;
}

static int chk_zerodiv(NodeBinop *b) {
    if(b->op != BIN_DIV)
        return 0;
    if(!node_is_number(b->right))
        return 0;

    NodeNumber *n = (NodeNumber *)b->right;
    if(!n->isfloat && (n->number == 0)) {
        return 1;
    }
    else if(n->fnumber == 0.0) {
        return 1;
    }

    return 0;
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

    CAST_AST(b)->ctype = res->ret;

    if(chk_zerodiv(b)) {
        error("zero division");
    }

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

        goto err;
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

    CAST_AST(u)->ctype = res->ret;

err:
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

    CAST_AST(a)->ctype = a->dst->ctype;

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

    CAST_AST(a)->ctype = a->dst->ctype;

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

    CAST_AST(a)->ctype = a->dst->ctype;

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
    if(!CAST_AST(s->ls)->ctype) return NULL;

    if(!CAST_AST(s->ls)->ctype->ptr) {
        error("cannot index into a value of type `%s`",
              s->ls->ctype->tostring(s->ls->ctype));
        return NULL;;
    }
    CAST_AST(s)->ctype = s->ls->ctype->ptr;

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
                Type *fieldty = CAST_AST((*left)->ctype->strct.field[i])->ctype;
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
        CAST_AST(d->memb)->ctype = CAST_AST(res)->ctype;
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
        CAST_AST(d->call)->ctype = CAST_AST(res)->ctype;
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
    CAST_AST(s)->ctype = s->tag;

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

    CAST_AST(b)->ctype = ((Ast *)b->cont->data[b->cont->len - 1])->ctype;

    return CAST_AST(b);
}

/*
static Ast *visit_nonscope_block(Ast *ast) {
    NodeBlock *b = (NodeBlock *)ast;
    for(int i = 0; i < b->cont->len; ++i) {
        b->cont->data[i] = visit(b->cont->data[i]);
    }

    return CAST_AST(b);
}*/

static Ast *visit_if(Ast *ast) {
    NodeIf *i = (NodeIf *)ast;
    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    CAST_AST(i)->ctype = mxcty_none;

    return CAST_AST(i);
}

static Ast *visit_exprif(Ast *ast) {
    NodeIf *i = (NodeIf *)ast;

    i->cond = visit(i->cond);
    i->then_s = visit(i->then_s);
    i->else_s = visit(i->else_s);

    if(!i->then_s || !i->else_s) return NULL;

    CAST_AST(i)->ctype = checktype(i->then_s->ctype, i->else_s->ctype);

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
        ((Ast *)f->vars->data[i])->ctype = f->iter->ctype->ptr;
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
        ((NodeFunction *)vec_last(fn_saver))->finfo.ftype->fnret;

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

        if(type_is(CAST_AST(v->var)->ctype, CTYPE_UNINFERRED)) {
            CAST_AST(v->var)->ctype = v->init->ctype;
        }
        else if(!checktype(CAST_AST(v->var)->ctype, v->init->ctype)) {
            if(!CAST_AST(v->var)->ctype) return NULL;

            error( "`%s` type is %s",
                    v->var->name,
                    CAST_AST(v->var)->ctype
                                    ->tostring(CAST_AST(v->var)->ctype));
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

    if(!type_is((*ast)->ctype, CTYPE_FUNCTION)) {
        if(!(*ast)->ctype) return NULL;

        error("`%s` is not function object",
              (*ast)->ctype->tostring((*ast)->ctype));
        return NULL;
    }

    if((*ast)->type == NDTYPE_VARIABLE) {
        /*
        if(((NodeVariable *)*ast)->is_overload) { */
            *ast = (Ast *)determine_overload((NodeVariable *)*ast,
                    argtys);
            /*
        }
        else {
            ;
        }*/
    }
    else {
        mxc_unimplemented("error");
        // TODO
    }

    if(!*ast) return NULL;

    NodeVariable *fn = (NodeVariable *)*ast;
    if(fn->finfo.isbuiltin) {
        return visit_bltinfn_call(self, ast, argtys);
    }
    self->ctype = fn->finfo.ftype->fnret;

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

    NodeVariable *func_var = determine_variable(fn->fnvar->name, scope);

    Vector *overload_fns;
    if(!func_var) {
        /* not registerd in scope */
        varlist_push(fnenv.current->vars, fn->fnvar);
        varlist_push(scope.current->vars, fn->fnvar);
        fn->fnvar->is_overload = false;
        overload_fns = fn->fnvar->children = New_Vector();
    }
    else {
        overload_fns = func_var->children;
        func_var->is_overload = true;
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
    for(int i = 0; i < fn->finfo.args->vars->len; ++i) {
        NodeVariable *cur = (NodeVariable *)fn->finfo.args->vars->data[i];
        cur->isglobal = false;

        CAST_AST(fn->fnvar)->ctype->fnarg->data[i] =
                solve_type(CAST_AST(fn->fnvar)->ctype->fnarg->data[i]);

        varlist_push(fnenv.current->vars, cur);
        varlist_push(scope.current->vars, cur);
    }

    vec_push(overload_fns, fn->fnvar);

    fn->block = visit(fn->block);
    if(!fn->block) return NULL;

    if(fn->block->type != NDTYPE_BLOCK) {
        // expr
        if(type_is(fn->finfo.ftype->fnret, CTYPE_UNINFERRED)) {
            fn->finfo.ftype->fnret = fn->block->ctype;
        }
        else {
            fn->finfo.ftype->fnret = solve_type(fn->finfo.ftype->fnret);

            Type *suc = checktype(fn->finfo.ftype->fnret, fn->block->ctype);

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
        if(!argtys->data[i]);
        else if(!(((Type *)argtys->data[i])->impl & TIMPL_SHOW)) {
            if(!argtys->data[i]) return NULL;

            error(
                "type %s does not implement `Show`",
                ((Type *)argtys->data[i])->tostring((Type *)argtys->data[i])
            );

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

    self->ctype = CAST_AST(fn)->ctype->fnret;

    return self;
}

static Ast *visit_load(Ast *ast) {
    NodeVariable *v = (NodeVariable *)ast;

    Ast *res = determine_variable(v->name, scope);

    if(!res) {
        error("undeclared variable: %s", v->name);
        return NULL;
    }

    v = res;

    CAST_AST(v)->ctype = solve_type(CAST_AST(v)->ctype);
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
    if(!var) return NULL;

    for(size_t i = 0; i < var->children->len; ++i) {
        NodeVariable *v = (NodeVariable *)var->children->data[i];

        if(CAST_AST(v)->ctype->fnarg->len == 0) {
            if(argtys->len == 0)
                return v;
            else
                continue;
        }

        if(CAST_TYPE(CAST_AST(v)->ctype->fnarg->data[0])->type ==
                CTYPE_ANY_VARARG) {
            return v;
        }

        if(CAST_AST(v)->ctype->fnarg->len != argtys->len) {
            continue;
        }

        if(CAST_TYPE(CAST_AST(v)->ctype->fnarg->data[0])->type ==
                CTYPE_ANY) {
            return v;
        }

        bool is_same = true;
        for(int i = 0; i < CAST_AST(v)->ctype->fnarg->len; ++i) {
            if(!checktype(CAST_TYPE(CAST_AST(v)->ctype->fnarg->data[i]),
                        CAST_TYPE(argtys->data[i]))) {
                is_same = false;
                break;
            }
        }

        if(is_same) return v;
    }
    error("Function not found: %s()", var->name);

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
        if(strncmp(ty1->strct.name, ty2->strct.name, strlen(ty1->strct.name)) == 0) {
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
