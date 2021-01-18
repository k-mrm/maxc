#include <string.h>
#include <stdlib.h>
#include "sema.h"
#include "ast.h"
#include "error/error.h"
#include "struct.h"
#include "lexer.h"
#include "parser.h"
#include "namespace.h"
#include "mlib.h"
#include "mlibapi.h"

enum acctype {
  VSTORE,
  VLOAD,
};

static Ast *visit(Ast *);

static NodeVariable *search_variable(char *, Scope *);
static NodeVariable *overload(NodeVariable *, Vector *, Scope *);
static Type *checktype_optional(Type *, Type *);

static Ast *visit_variable(Ast *ast, enum acctype acc);

static Scope *scope;
static Vector *fn_saver;
static Vector *iter_saver;
static int loop_nest = 0;

int ngvar = 0;

static Ast *visit_list_with_size(NodeList *l) {
  l->nelem = visit(l->nelem); 
  if(!l->nelem) return NULL;
  if(!checktype(l->nelem->ctype, mxc_int)) {
    error("Number of elements in array must be numeric");
    return NULL;
  }

  l->init = visit(l->init);
  if(!l->init) return NULL;
  l->init->ctype = solvetype(l->init->ctype);
  CTYPE(l) = new_type_list(l->init->ctype);

  return CAST_AST(l);
}

static Ast *visit_list(Ast *ast) {
  NodeList *l = (NodeList *)ast;
  if(l->nelem) {
    return visit_list_with_size(l);
  }

  Type *base = NULL;

  if(l->nsize == 0) {
    base = new_type(CTYPE_UNSOLVED);
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

        errline(el->lineno, "expect `%s`, found `%s`", typefmt(base), typefmt(el->ctype));
        return NULL;
      }
    }
  }

  CTYPE(l) = new_type_list(base);

  return (Ast *)l;
}

static Ast *visit_hashtable(Ast *ast) {
  NodeHashTable *t = (NodeHashTable *)ast;
  Type *tkey = NULL;
  Type *tval = NULL;

  for(int i = 0; i < t->key->len; i++) {
    t->key->data[i] = visit(t->key->data[i]);
    t->val->data[i] = visit(t->val->data[i]);
    tkey = CTYPE(t->key->data[i]);
    tval = CTYPE(t->val->data[i]);
  }

  CTYPE(t) = new_type_table(tkey, tval);

  return (Ast *)t;
}

static Ast *visit_binary(Ast *ast) {
  NodeBinop *b = (NodeBinop *)ast;

  b->left = visit(b->left);
  b->right = visit(b->right);

  if(!b->left || !b->left->ctype) return NULL;
  if(!b->right || !b->right->ctype) return NULL;

  Type *res = operator_type(OPE_BINARY, b->op, prune(b->left->ctype), prune(b->right->ctype));

  if(!res) {
    errline(ast->lineno, "undefined binary operation: `%s` %s `%s`",
        typefmt(b->left->ctype),
        operator_dump(OPE_BINARY, b->op),
        typefmt(b->right->ctype));

    goto err;
  }

  CTYPE(b) = res;

err:
  return CAST_AST(b);
}

static Ast *visit_unary(Ast *ast) {
  NodeUnaop *u = (NodeUnaop *)ast;

  u->expr = visit(u->expr);
  if(!u->expr || !u->expr->ctype) return NULL;

  Type *res = operator_type(OPE_UNARY, u->op, u->expr->ctype, NULL);

  if(!res) {
    errline(ast->lineno, "undefined unary operation `%s` to `%s`",
        operator_dump(OPE_UNARY, u->op), typefmt(u->expr->ctype));
    return NULL;
  }

  CTYPE(u) = res;

  return CAST_AST(u);
}

static Ast *visit_var_assign(NodeAssignment *a) {
  a->dst = visit_variable(a->dst, VSTORE);
  NodeVariable *v = (NodeVariable *)a->dst;

  if(v->vattr & VARATTR_CONST) {
    error("assignment of read-only variable: %s", v->name);
    return NULL;
  }
  v->vattr &= ~VARATTR_UNINIT;

  if(!checktype(a->dst->ctype, a->src->ctype)) {
    if(!a->dst->ctype || !a->src->ctype) return NULL;

    errline(LINENO(a), "type error `%s`, `%s`", typefmt(a->dst->ctype), typefmt(a->src->ctype));
    return NULL;
  }

  CTYPE(a) = a->dst->ctype;

  return CAST_AST(a);
}

static Ast *visit_subscr_assign(NodeAssignment *a) {
  a->dst = visit(a->dst);
  if(!checktype(a->dst->ctype, a->src->ctype)) {
    if(!a->dst->ctype || !a->src->ctype)
      return NULL;

    errline(LINENO(a), "type error `%s`, `%s`", typefmt(a->dst->ctype), typefmt(a->src->ctype));
    return NULL;
  }

  CTYPE(a) = a->dst->ctype;

  return CAST_AST(a);
}

static Ast *visit_member_assign(NodeAssignment *a) {
  a->dst = visit(a->dst);
  if(!checktype(a->dst->ctype, a->src->ctype)) {
    if(!a->dst->ctype || !a->src->ctype)
      return NULL;

    errline(LINENO(a), "type error `%s`, `%s`", typefmt(a->dst->ctype), typefmt(a->src->ctype));
    return NULL;
  }

  CTYPE(a) = a->dst->ctype;

  return CAST_AST(a);
}

static Ast *visit_assign(Ast *ast) {
  NodeAssignment *a = (NodeAssignment *)ast;
  a->src = visit(a->src);
  if(!a->dst || !a->src) return NULL;

  switch(a->dst->type) {
    case NDTYPE_VARIABLE:   return visit_var_assign(a);
    case NDTYPE_SUBSCR:     return visit_subscr_assign(a);
    case NDTYPE_DOTEXPR: {
      if(((NodeDotExpr *)a->dst)->t.member)
        return visit_member_assign(a);
      __attribute__((fallthrough));
    }
    default: {
      errline(ast->lineno, "left side of the expression is not valid");
      return NULL;
    }
  }
}

static Ast *visit_subscr(Ast *ast) {
  NodeSubscript *s = (NodeSubscript *)ast;

  s->ls = visit(s->ls);
  s->index = visit(s->index);

  if(!s->ls) return NULL;
  if(!CTYPE(s->ls)) return NULL;
  if(!s->index) return NULL;
  if(!CTYPE(s->index)) return NULL;

  if(!is_subscriptable(CTYPE(s->ls))) {
    errline(LINENO(s), "cannot apply subscription to `%s`", typefmt(CTYPE(s->ls)));
    return NULL;
  }

  if(!checktype(CTYPE(s->index), CTYPE(s->ls)->key)) {
    errline(s->index->lineno, "expected `%s`, found `%s`",
        typefmt(CTYPE(s->ls)->key), typefmt(CTYPE(s->index)));
    return NULL;
  }

  if(!CTYPE(s->ls)->val) {
    error("cannot index into a value of type `%s`", typefmt(s->ls->ctype));
    return NULL;
  }

  CTYPE(s) = s->ls->ctype->val;

  return (Ast *)s;
}

static Ast *visit_member_impl(Ast *self, Ast **left, Ast **right) {
  if(!*left || !(*left)->ctype)
    return NULL;

  if(!is_struct((*left)->ctype))
    return NULL;

  if((*right)->type == NDTYPE_VARIABLE) {
    NodeVariable *rhs = (NodeVariable *)*right;
    size_t nfield = (*left)->ctype->strct.nfield;

    for(size_t i = 0; i < nfield; ++i) {
      if(strncmp((*left)->ctype->strct.field[i]->name,
            rhs->name,
            strlen((*left)->ctype->strct.field[i]->name)) == 0) {
        Type *fieldty = CTYPE((*left)->ctype->strct.field[i]);
        self->ctype = solvetype(fieldty);
        return self;
      }
    }
  }

  return NULL;
}

static Ast *visit_fncall_impl(Ast *self, Ast **func, Vector *args) {
  Vector *argtys = new_vector();
  for(int i = 0; i < args->len; ++i) {
    if(args->data[i])
      vec_push(argtys, CAST_AST(args->data[i])->ctype);
  }

  if(!*func) return NULL;

  if(!type_is(CTYPE(*func), CTYPE_FUNCTION) && !type_is(CTYPE(*func), CTYPE_GENERATOR)) {
    if(CTYPE(*func))
      errline(LINENO(self), "`%s` is not function or iterator object", typefmt(CTYPE(*func)));
    return NULL;
  }

  if((*func)->type == NDTYPE_VARIABLE) {
    NodeVariable *v_func = (NodeVariable *)*func;
    Ast *ret = (Ast *)overload(v_func, argtys, scope);
    if(!ret) {
      errline(LINENO(self), "unknown function: %s(%s)", v_func->name, vec_tyfmt(argtys));
    }
    *func = ret;
  }
  else {
    mxc_unimplemented("error");
    // TODO
  }

  if(!*func) return NULL;

  NodeVariable *fn = (NodeVariable *)*func;
  self->ctype = CTYPE(fn)->fnret;

  return self;
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
    d->memb = node_member(d->left, d->right, d->right->lineno);
    CTYPE(d->memb)= CTYPE(res);
    return CAST_AST(d);
  }

  d->right = visit(d->right);
  Vector *arg = new_vector_capa(1);
  vec_push(arg, d->left);
  res = (NodeDotExpr *)visit_fncall_impl((Ast *)d, &d->right, arg);
  if(res) {
    d = res;
    d->t.fncall = 1;
    d->call = node_fncall(d->right, arg, d->left->lineno);
    CTYPE(d->call)= CTYPE(res);
    return CAST_AST(d);
  }

  return NULL;
}

static Ast *visit_object(Ast *ast) {
  NodeObject *s = (NodeObject *)ast;

  mxc_assert(CAST_AST(s->decls->data[0])->type == NDTYPE_VARIABLE,
      "internal error");

  MxcStruct struct_info = new_cstruct(s->tagname, (NodeVariable **)s->decls->data, s->decls->len);

  vec_push(scope->userdef_type, new_type_struct(struct_info));

  return CAST_AST(s);
}

static Ast *visit_struct_init(Ast *ast) {
  NodeStructInit *s = (NodeStructInit *)ast;

  s->tag = solvetype(s->tag);
  CTYPE(s) = s->tag;

  for(int i = 0; s->fields && i < s->fields->len; ++i) {
    /* TODO */
  }
  for(int i = 0; s->inits && i < s->inits->len; ++i) {
    s->inits->data[i] = visit(s->inits->data[i]);
  }

  return CAST_AST(s);
}

static Ast *visit_block(Ast *ast) {
  NodeBlock *b = (NodeBlock *)ast;
  scope = make_scope(scope, BLOCKSCOPE);

  for(int i = 0; i < b->cont->len; ++i) {
    b->cont->data[i] = visit(b->cont->data[i]);
  }

  scope = scope_escape(scope);

  return CAST_AST(b);
}

static Ast *visit_typed_block(Ast *ast) {
  NodeBlock *b = (NodeBlock *)ast;
  scope = make_scope(scope, BLOCKSCOPE);

  for(int i = 0; i < b->cont->len; ++i) {
    b->cont->data[i] = visit(b->cont->data[i]);
  }

  scope = scope_escape(scope);
  CTYPE(b)= CTYPE(b->cont->data[b->cont->len - 1]);

  return CAST_AST(b);
}

static Ast *visit_if(Ast *ast) {
  NodeIf *i = (NodeIf *)ast;
  i->cond = visit(i->cond);
  i->then_s = visit(i->then_s);
  i->else_s = visit(i->else_s);

  CTYPE(i) = mxc_none;

  return CAST_AST(i);
}

static Ast *visit_exprif(Ast *ast) {
  NodeIf *i = (NodeIf *)ast;
  i->cond = visit(i->cond);
  i->then_s = visit(i->then_s);
  i->else_s = visit(i->else_s);

  if(!i->then_s || !i->else_s) return NULL;

  CTYPE(i) = checktype(i->then_s->ctype, i->else_s->ctype);

  return CAST_AST(i);
}

static Ast *visit_fncall(Ast *ast) {
  NodeFnCall *f = (NodeFnCall *)ast;
  f->func = visit(f->func);
  for(size_t i = 0; i < f->args->len; ++i) {
    f->args->data[i] = visit((Ast *)f->args->data[i]);
  }

  return visit_fncall_impl((Ast *)f, &f->func, f->args);
}

static bool is_iterable_node(Ast *node) {
  return is_iterable(node->ctype);
}

static Type *loop_variable_type(Type *t) {
  if(t->type == CTYPE_ITERATOR) {
    return t->ptr;
  }
  else {
    return t->val;
  }
}

static Ast *visit_for(Ast *ast) {
  NodeFor *f = (NodeFor *)ast;
  f->iter = visit(f->iter);
  if(!f->iter) return NULL;

  if(!is_iterable_node(f->iter)) {
    if(!f->iter->ctype) return NULL;

    errline(f->iter->lineno, "`%s` is not an iterable object", typefmt(f->iter->ctype));
    return NULL;
  }

  bool isglobal = fscope_isglobal(scope);

  scope = make_scope(scope, BLOCKSCOPE);

  for(int i = 0; i < f->vars->len; i++) {
    CTYPE(f->vars->data[i]) = loop_variable_type(f->iter->ctype);
    ((NodeVariable *)f->vars->data[i])->isglobal = isglobal;

    scope_push_var(scope, f->vars->data[i]);
    f->vars->data[i] = visit(f->vars->data[i]);
  }

  loop_nest++;
  f->body = visit(f->body);
  loop_nest--;

  scope = scope_escape(scope);

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

static Ast *visit_switch(Ast *ast) {
  NodeSwitch *s = (NodeSwitch *)ast;
  s->match = visit(s->match);
  s->eelse = visit(s->eelse);

  for(int i = 0; i < s->ecase->len; i++) {
    Ast *ec = s->ecase->data[i] = visit(s->ecase->data[i]);
    Ast *b = s->body->data[i] = visit(s->body->data[i]);
    if(!checktype(CTYPE(s->match), CTYPE(ec))) {
      errline(LINENO(ec), "type checking failed");
      return NULL;
    }
  }

  return (Ast *)s;
}

static Ast *visit_yield(Ast *ast) {
  NodeYield *y = (NodeYield *)ast;
  y->cont = visit(y->cont);
  if(!y->cont)  return NULL;

  if(iter_saver->len == 0) {
    errline(ast->lineno, "use of yield statement outside iterator block");
    return NULL;
  }

  Type *cur_retty = CTYPE(((NodeFunction *)vec_last(iter_saver))->fnvar)->fnret->ptr;

  if(!checktype(cur_retty, y->cont->ctype)) {
    if(!cur_retty || !y->cont->ctype) return NULL;

    error("type error: expected %s, found %s",
        typefmt(cur_retty), typefmt(y->cont->ctype));
  }

  return (Ast *)y;
}

static Ast *visit_return(Ast *ast) {
  NodeReturn *r = (NodeReturn *)ast;
  r->cont = visit(r->cont);
  if(!r->cont) return NULL;

  if(fn_saver->len == 0) {
    errline(ast->lineno, "use of return statement outside function or block");
    return NULL;
  }

  Type *cur_fn_retty =
    CTYPE(((NodeFunction *)vec_last(fn_saver))->fnvar)->fnret;

  if(!checktype(cur_fn_retty, r->cont->ctype)) {
    if(!cur_fn_retty || !r->cont->ctype) return NULL;

    error("type error: expected %s, found %s",
        typefmt(cur_fn_retty),
        typefmt(r->cont->ctype));
  }

  return (Ast *)r;
}

static Ast *visit_break(Ast *ast) {
  NodeBreak *b = (NodeBreak *)ast;
  if(loop_nest == 0) {
    errline(ast->lineno, "break statement must be inside loop statement");
    return NULL;
  }

  return (Ast *)b;
}

static Ast *visit_skip(Ast *ast) {
  NodeSkip *s = (NodeSkip *)ast;
  if(loop_nest == 0) {
    errline(ast->lineno, "skip statement must be inside loop statement");
    return NULL;
  }

  return (Ast *)s;
}

static Ast *visit_vardecl(Ast *);

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

  v->var->isglobal = fscope_isglobal(scope);

  if(v->init) {
    v->init = visit(v->init);
    if(!v->init) return NULL;

    if(type_is(CTYPE(v->var), CTYPE_UNINFERRED)) {
      if(unsolved(v->init->ctype)) {
        errline(LINENO(v), "specify type explictly");
      }
      else {
        CTYPE(v->var) = v->init->ctype;
      }
    }
    else if(unsolved(v->init->ctype)) {
      v->init->ctype = CTYPE(v->var);
    }
    else if(!checktype(CTYPE(v->var), v->init->ctype)) {
      if(!CTYPE(v->var)) return NULL;

      errline(LINENO(v->var), "`%s` type is %s", v->var->name, typefmt(CTYPE(v->var)));
      return NULL;
    }
  }
  else {
    if(type_is(CAST_AST(v->var)->ctype, CTYPE_UNINFERRED)) {
      error("must always be initialized when doing type inference");
      return NULL;
    }

    CAST_AST(v->var)->ctype = solvetype(CAST_AST(v->var)->ctype);

    v->var->vattr |= VARATTR_UNINIT;
  }

  if(chk_var_conflict(scope, v->var)) {
    errline(LINENO(v), "conflict variable declaration `%s`", v->var->name);
    return NULL;
  }
  scope_push_var(scope, v->var);

  return CAST_AST(v);
}

static Ast *visit_funcdef(Ast *ast, bool iter) {
  NodeFunction *fn = (NodeFunction *)ast;

  vec_push(iter? iter_saver : fn_saver, fn);
  fn->fnvar->isglobal = fscope_isglobal(scope);

  scope_push_var(scope, fn->fnvar);

  scope = make_scope(scope, FUNCSCOPE);

  fn->fnvar->vattr = 0;

  /* register argument(s) in this scope */
  for(int i = 0; i < fn->args->len; ++i) {
    NodeVariable *curv = (NodeVariable *)fn->args->data[i];
    curv->isglobal = false;

    CTYPE(fn->fnvar)->fnarg->data[i] =
      solvetype(CTYPE(fn->fnvar)->fnarg->data[i]);

    scope_push_var(scope, curv);
  }

  fn->block = visit(fn->block);
  if(!fn->block) return NULL;

  if(fn->block->type != NDTYPE_BLOCK) {
    /* expr */
    if(type_is(CTYPE(fn->fnvar)->fnret, CTYPE_UNINFERRED)) {
      CTYPE(fn->fnvar)->fnret = fn->block->ctype;
    }
    else {
      CTYPE(fn->fnvar)->fnret = solvetype(CTYPE(fn->fnvar)->fnret);
      Type *suc = checktype(CTYPE(fn->fnvar)->fnret, fn->block->ctype);
      if(!suc) {
        error("return type error");
        return NULL;
      }
    }
  }

  var_assign_id(scope);

  fn->lvars = scope->fscope_vars;

  scope = scope_escape(scope);

  vec_pop(iter? iter_saver : fn_saver);

  return CAST_AST(fn);
}

static Ast *visit_variable(Ast *ast, enum acctype acc) {
  NodeVariable *v = (NodeVariable *)ast;
  NodeVariable *res = search_variable(v->name, scope);
  if(!res) {
    errline(ast->lineno, "undeclared variable: %s", v->name);
    return NULL;
  }
  v = res;

  CTYPE(v) = solvetype(CTYPE(v));
  if(acc == VLOAD && (v->vattr & VARATTR_UNINIT) && !type_is(CAST_AST(v)->ctype, CTYPE_STRUCT)) {
    errline(ast->lineno, "use of uninit variable: %s", v->name);
    return NULL;
  }

  v->used = true;

  return CAST_AST(v);
}

static Ast *visit_namespace(Ast *ast) {
  NodeNameSpace *s = (NodeNameSpace *)ast;
  // scope = make_scope(scope, BLOCKSCOPE);

  for(int i = 0; i < s->block->cont->len; i++) {
    s->block->cont->data[i] = visit(s->block->cont->data[i]);
  }

  // reg_namespace(s->name, scope->vars);

  // scope = scope_escape(scope);

  return CAST_AST(s);
}

static Ast *visit_assert(Ast *ast) {
  NodeAssert *a = (NodeAssert *)ast;
  a->cond = visit(a->cond);
  if(!a->cond) return NULL;

  if(!checktype(a->cond->ctype, mxc_bool)) {
    errline(ast->lineno, "assert conditional expression type must be"
        "`bool`, but got %s", typefmt(a->cond->ctype));

    return NULL;
  }

  return (Ast *)a;
}

static Ast *visit_modfn_call(Ast *ast) {
  NodeModuleFuncCall *v = (NodeModuleFuncCall *)ast;
  NodeVariable *ns_name = (NodeVariable *)v->name;
  Vector *vars = search_namespace(ns_name->name);
  if(!vars) {
    error("unknown namespace: `%s`", ns_name->name);
    return NULL;
  }
  NodeVariable *id = (NodeVariable *)v->ident;

  for(size_t i = 0; i < vars->len; ++i) {
    NodeVariable *cur = (NodeVariable *)vars->data[i];

    if(strcmp(id->name, cur->name) == 0) {
      return (Ast *)cur;
    }
  }

  error("undeclared variable: `%s` in %s", id->name, ns_name->name);
  return NULL;
}

static NodeVariable *search_variable(char *name, Scope *sc) {
  for(Scope *s = sc; ; s = s->parent) {
    for(int i = 0; i < s->vars->len; ++i) {
      if(strcmp(((NodeVariable *)s->vars->data[i])->name, name) == 0) {
        return (NodeVariable *)s->vars->data[i];
      }
    }

    if(scope_isglobal(s))
      break;
  }
  /* Not Found */
  return NULL;
}

static NodeVariable *overload(NodeVariable *v, Vector *argtys, Scope *scp) {
  char *fname = v->name;
  for(Scope *s = scp; ; s = s->parent) {
    for(int i = 0; i < s->vars->len; ++i) {
      NodeVariable *curv = (NodeVariable *)s->vars->data[i];

      if(strcmp(curv->name, fname) != 0)
        continue;

      if(CTYPE(curv)->fnarg->len == 0) {
        if(argtys->len == 0) return curv;
        else continue;
      }

      if(((Type *)CTYPE(curv)->fnarg->data[0])->type == CTYPE_ANY_VARARG)
        return curv;

      if(CTYPE(curv)->fnarg->len != argtys->len)
        continue;

      if(((Type *)CTYPE(curv)->fnarg->data[0])->type == CTYPE_ANY)
        return curv;

      bool matched = true;
      for(int argi = 0; argi < CTYPE(curv)->fnarg->len; argi++) {
        if(!checktype((Type *)CTYPE(curv)->fnarg->data[argi], (Type *)argtys->data[argi])) {
          matched = false;
          break;
        }
      }

      bool exist_tvar = false;
      if(matched) {
        Type *t_newv = typedup(CTYPE(curv));

        for(int a = 0; a < CTYPE(curv)->fnarg->len; a++) {
          Type *t = (Type *)CTYPE(curv)->fnarg->data[a];
          exist_tvar = exist_tvar || has_tvar(t);
        }

        if(exist_tvar) {
          NodeVariable *newv = malloc(sizeof(NodeVariable));
          *newv = *curv;
          CTYPE(newv) = t_newv;
          return newv;
        }
        else {
          free(t_newv);
          return curv;
        }
      }
    }

    if(scope_isglobal(s))
      break;
  }

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

Type *solvetype(Type *ty) {
  if(!unsolved(ty)) return ty;

  for(Scope *s = scope; ; s = s->parent) {
    for(int i = 0; i < s->userdef_type->len; ++i) {
      Type *ud = (Type *)s->userdef_type->data[i];

      if(type_is(ud, CTYPE_STRUCT)) {
        if(strcmp(ud->strct.name, ty->name) == 0) {
          return ud;
        }
      }
    }

    if(scope_isglobal(s))
      break;
  }

  error("undefined type: %s", ty->name);
  return ty;
}

static Ast *visit(Ast *ast) {
  if(!ast)
    return NULL;

  switch(ast->type) {
    case NDTYPE_NUM:
    case NDTYPE_BOOL:
    case NDTYPE_NULL:
    case NDTYPE_STRING: break;
    case NDTYPE_LIST: return visit_list(ast);
    case NDTYPE_HASHTABLE: return visit_hashtable(ast);
    case NDTYPE_SUBSCR: return visit_subscr(ast);
    case NDTYPE_TUPLE: mxc_unimplemented("tuple"); return ast;
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
    case NDTYPE_SWITCH: return visit_switch(ast);
    case NDTYPE_BLOCK: return visit_block(ast);
    case NDTYPE_TYPEDBLOCK: return visit_typed_block(ast);
    case NDTYPE_RETURN: return visit_return(ast);
    case NDTYPE_YIELD: return visit_yield(ast);
    case NDTYPE_BREAK: return visit_break(ast);
    case NDTYPE_SKIP: return visit_skip(ast);
    case NDTYPE_BREAKPOINT: break;
    case NDTYPE_VARIABLE: return visit_variable(ast, VLOAD);
    case NDTYPE_FUNCCALL: return visit_fncall(ast);
    case NDTYPE_FUNCDEF: return visit_funcdef(ast, false);
    case NDTYPE_ITERATOR: return visit_funcdef(ast, true);
    case NDTYPE_VARDECL: return visit_vardecl(ast);
    case NDTYPE_NAMESPACE: return visit_namespace(ast);
    case NDTYPE_MODULEFUNCCALL: return visit_modfn_call(ast);
    case NDTYPE_ASSERT: return visit_assert(ast);
    case NDTYPE_NONENODE: break;
    default: mxc_assert(0, "unimplemented node");
  }

  return ast;
}

SemaResult sema_analysis_repl(Vector *ast) {
  ast->data[0] = visit((Ast *)ast->data[0]);
  Ast *stmt = (Ast *)ast->data[0];

  var_assign_id(scope);

  scope_escape(scope);

  bool isexpr = ast_isexpr(stmt);
  char *typestr;
  if(isexpr && stmt && stmt->ctype) {
    typestr = typefmt(stmt->ctype);
  }
  else {
    typestr = "";
  }

  return (SemaResult){ isexpr, typestr };
}

static void setup_bltin(void) {
  int vid = 0;
  for(int i = 0; i < gmodule->len; ++i) {
    Vector *a = ((MxcModule *)gmodule->data[i])->cimpl;
    for(int j = 0; j < a->len; j++) {
      NodeVariable *v = ((MCimpl *)a->data[j])->var;
      v->isglobal = true;
      v->vid = vid++;
      scope_push_var(scope, v);
    }
  }
}

void sema_init() {
  scope = make_scope(NULL, FUNCSCOPE);
  fn_saver = new_vector();
  iter_saver = new_vector();
  setup_bltin();
}

int sema_analysis(Vector *ast) {
  for(int i = 0; i < ast->len; ++i) {
    ast->data[i] = visit((Ast *)ast->data[i]);
  }

  ngvar += var_assign_id(scope);

  scope_escape(scope);

  return ngvar;
}
