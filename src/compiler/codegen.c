#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "maxc.h"
#include "ast.h"
#include "codegen.h"
#include "bytecode.h"
#include "error/error.h"
#include "literalpool.h"
#include "function.h"
#include "mlibapi.h"
#include "object/mstr.h"

struct cgen *newcgen_glb(int ngvars) {
  struct cgen *n = malloc(sizeof(struct cgen));
  n->iseq = new_bytecode();
  n->gvars = malloc(sizeof(MxcValue) * ngvars);
  n->ngvars = ngvars;
  n->ltable = new_vector();
  n->loopstack = new_vector();
  n->d = new_debuginfo(filename, "<global>");

  return n;
}

struct cgen *newcgen(struct cgen *p, char *fname) {
  struct cgen *n = malloc(sizeof(struct cgen));
  n->iseq = new_bytecode();
  n->gvars = p->gvars;
  n->ngvars = p->ngvars;
  n->ltable = p->ltable;
  n->loopstack = p->loopstack;
  n->d = new_debuginfo(filename, fname);

  return n;
}

static void cpush(struct cgen *c, uint8_t a, int lineno) {
  vec_push(c->d->pc_line_map, lineno);
  push(c->iseq, a);
}

#define DEF_CPUSH(byte) \
  void cpush ## byte(struct cgen *c, enum OPCODE op, int ## byte ## _t a, int lineno) { \
    for(int i = 0; i < (byte) / 8 + 1; i++) {                                           \
      vec_push(c->d->pc_line_map, lineno);                                              \
    }                                                                                   \
    push ## byte(c->iseq, op, a);                                                       \
  }

DEF_CPUSH(8)
DEF_CPUSH(32)

static void gen(struct cgen *, Ast *, bool);

static void emit_rawobject(struct cgen *c, MxcValue ob, bool use_ret) {
  int key = lpool_push_object(c->ltable, ob);
  cpush32(c, OP_PUSH, key, -1);

  if(!use_ret)
    cpush(c, OP_POP, -1);
}

static void emit_store(struct cgen *c, Ast *ast, bool use_ret) {
  NodeVariable *v = (NodeVariable *)ast;

  cpush32(c, v->isglobal? OP_STORE_GLOBAL : OP_STORE_LOCAL, v->vid, ast->lineno);

  if(!use_ret)
    cpush(c, OP_POP, ast->lineno);
}

static void emit_builtins(MInterp *interp, struct cgen *c) {
  for(size_t i = 0; i < interp->module->len; ++i) {
    MxcModule *mod = (MxcModule *)interp->module->data[i];
    log_dbg("load %s\n", mod->name);

    for(int j = 0; j < mod->cimpl->len; j++) {
      MCimpl *cimpl = (MCimpl *)mod->cimpl->data[j];
      c->gvars[cimpl->var->vid] = cimpl->impl;
    }
  }
}

static void emit_num(struct cgen *c, Ast *ast, bool use_ret) {
  NodeNumber *n = (NodeNumber *)ast;
  int lno = ast->lineno;

  if(isflo(n->value)) {
    int key = lpool_push_float(c->ltable, n->value.fnum);
    cpush32(c, OP_FPUSH, key, lno);
  }
  else if(isobj(n->value)) {
    emit_rawobject(c, n->value, true);
  }
  else if(n->value.num > INT_MAX) {
    int key = lpool_push_long(c->ltable, n->value.num);
    cpush32(c, OP_LPUSH, key, lno);
  }
  else {
    switch(n->value.num) {
      case 0:     cpush(c, OP_PUSHCONST_0, lno); break;
      case 1:     cpush(c, OP_PUSHCONST_1, lno); break;
      case 2:     cpush(c, OP_PUSHCONST_2, lno); break;
      case 3:     cpush(c, OP_PUSHCONST_3, lno); break;
      default:    cpush32(c, OP_IPUSH, n->value.num, lno);  break;
    }
  }

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_bool(struct cgen *c, Ast *ast, bool use_ret) {
  NodeBool *b = (NodeBool *)ast;
  int lno = ast->lineno;
  cpush(c->iseq, (b->boolean ? OP_PUSHTRUE : OP_PUSHFALSE), lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_null(struct cgen *c, Ast *ast, bool use_ret) {
  INTERN_UNUSE(ast);
  int lno = ast->lineno;
  cpush(c, OP_PUSHNULL, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_string(struct cgen *c, Ast *ast, bool use_ret) {
  int key = lpool_push_str(c->ltable, ((NodeString *)ast)->string);
  int lno = ast->lineno;
  cpush32(c, OP_STRINGSET, key, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_list_with_size(struct cgen *c, NodeList *l, bool use_ret) {
  gen(c, l->init, true);
  gen(c, l->nelem, true);
  int lno = LINENO(l);

  cpush(c, OP_LISTSET_SIZE, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_list(struct cgen *c, Ast *ast, bool use_ret) {
  NodeList *l = (NodeList *)ast;
  int lno = LINENO(ast);
  if(l->nelem) {
    return emit_list_with_size(c, l, use_ret);
  }

  for(int i = l->nsize; i >= 0; i--) {
    gen(c, (Ast *)l->elem->data[i], true);
  }

  cpush32(c, OP_LISTSET, l->nsize, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_hashtable(struct cgen *c, Ast *ast, bool use_ret) {
  NodeHashTable *t = (NodeHashTable *)ast;
  int lno = LINENO(ast);

  for(int i = t->key->len - 1; i >= 0; i--) {
    gen(c, (Ast *)t->key->data[i], true);
    gen(c, (Ast *)t->val->data[i], true);
  }

  push32(c->iseq, OP_TABLESET, t->key->len);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_struct_init(struct cgen *c, Ast *ast, bool use_ret) {
  NodeStructInit *s = (NodeStructInit *)ast;

  push32(c->iseq, OP_STRUCTSET, CAST_AST(s)->ctype->strct.nfield);

  // TODO

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_subscr(struct cgen *c, Ast *ast) {
  NodeSubscript *l = (NodeSubscript *)ast;

  gen(c, l->index, true);
  gen(c, l->ls, true);

  push_0arg(c->iseq, OP_SUBSCR);
}

static void emit_tuple(struct cgen *c, Ast *ast) {
  NodeTuple *t = (NodeTuple *)ast;

  for(int i = (int)t->nsize - 1; i >= 0; i--)
    gen(c, (Ast *)t->exprs->data[i], true);
}

static void emit_catch(struct cgen *c, NodeBinop *b, bool use_ret) {
  push_0arg(c->iseq, OP_TRY);

  gen(c, b->left, true);
  size_t catch_pos = c->iseq->len;
  push32(c->iseq, OP_CATCH, 0);
  gen(c, b->right, true);

  replace_int32(catch_pos, c->iseq, c->iseq->len);
  
  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_binop(struct cgen *c, Ast *ast, bool use_ret) {
  NodeBinop *b = (NodeBinop *)ast;

  if(b->op == BIN_QUESTION)
    return emit_catch(c, b, use_ret);

  gen(c, b->left, true);
  gen(c, b->right, true);

  if(type_is(b->left->ctype, CTYPE_INT)) {
    switch(b->op) {
      case BIN_ADD: push_0arg(c->iseq, OP_ADD); break;
      case BIN_SUB: push_0arg(c->iseq, OP_SUB); break;
      case BIN_MUL: push_0arg(c->iseq, OP_MUL); break;
      case BIN_DIV: push_0arg(c->iseq, OP_DIV); break;
      case BIN_MOD: push_0arg(c->iseq, OP_MOD); break;
      case BIN_EQ: push_0arg(c->iseq, OP_EQ); break;
      case BIN_NEQ: push_0arg(c->iseq, OP_NOTEQ); break;
      case BIN_LOR: push_0arg(c->iseq, OP_LOGOR); break;
      case BIN_LAND: push_0arg(c->iseq, OP_LOGAND); break;
      case BIN_LT: push_0arg(c->iseq, OP_LT); break;
      case BIN_LTE: push_0arg(c->iseq, OP_LTE); break;
      case BIN_GT: push_0arg(c->iseq, OP_GT); break;
      case BIN_GTE: push_0arg(c->iseq, OP_GTE); break;
      case BIN_BXOR: push_0arg(c->iseq, OP_BXOR); break;
      default:    break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_FLOAT)){
    switch(b->op) {
      case BIN_ADD: push_0arg(c->iseq, OP_FADD); break;
      case BIN_SUB: push_0arg(c->iseq, OP_FSUB); break;
      case BIN_MUL: push_0arg(c->iseq, OP_FMUL); break;
      case BIN_DIV: push_0arg(c->iseq, OP_FDIV); break;
      case BIN_MOD: push_0arg(c->iseq, OP_FMOD); break;
      case BIN_EQ: push_0arg(c->iseq, OP_FEQ); break;
      case BIN_NEQ: push_0arg(c->iseq, OP_FNOTEQ); break;
      case BIN_LOR: push_0arg(c->iseq, OP_FLOGOR); break;
      case BIN_LAND: push_0arg(c->iseq, OP_FLOGAND); break;
      case BIN_LT: push_0arg(c->iseq, OP_FLT); break;
      case BIN_LTE: push_0arg(c->iseq, OP_FLTE); break;
      case BIN_GT: push_0arg(c->iseq, OP_FGT); break;
      case BIN_GTE: push_0arg(c->iseq, OP_FGTE); break;
      default:    break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_STRING)){
    switch(b->op) {
      case BIN_ADD: push_0arg(c->iseq, OP_STRCAT); break;
      case BIN_EQ: {
        emit_rawobject(c, new_cfunc(mstr_eq), true);
        push32(c->iseq, OP_CALL, 2);
      }
      default: break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_BOOL)) {
    switch(b->op) {
      case BIN_LOR: push_0arg(c->iseq, OP_LOGOR); break;
      case BIN_LAND: push_0arg(c->iseq, OP_LOGAND); break;
      case BIN_EQ: push_0arg(c->iseq, OP_EQ); break;
      case BIN_NEQ: push_0arg(c->iseq, OP_NOTEQ); break;
      default: break;
    }
  }

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_member(struct cgen *c, Ast *ast, bool use_ret) {
  NodeMember *m = (NodeMember *)ast;

  gen(c, m->left, true);
  NodeVariable *rhs = (NodeVariable *)m->right;

  size_t i = 0;
  for(; i < m->left->ctype->strct.nfield; ++i) {
    if(strcmp(m->left->ctype->strct.field[i]->name, rhs->name) == 0) {
      break;
    }
  }
  push32(c->iseq, OP_MEMBER_LOAD, i);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_fncall(struct cgen *c, Ast *ast, bool use_ret) {
  NodeFnCall *f = (NodeFnCall *)ast;

  for(int i = 0; i < f->args->len; i++)
    gen(c, (Ast *)f->args->data[i], true);
  gen(c, f->func, true);

  push32(c->iseq, OP_CALL, f->args->len);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_dotexpr(struct cgen *c, Ast *ast, bool use_ret) {
  NodeDotExpr *d = (NodeDotExpr *)ast;

  if(d->t.member) {
    emit_member(c, (Ast *)d->memb, use_ret);
  }
  else if(d->t.fncall) {
    emit_fncall(c, (Ast *)d->call, use_ret);
  }
  else {
    /* unreachable */
  }
}

static void emit_unary_neg(struct cgen *c, NodeUnaop *u) {
  if(type_is(((Ast *)u)->ctype, CTYPE_INT)) {
    push_0arg(c->iseq, OP_INEG);
  }
  else {  // float
    push_0arg(c->iseq, OP_FNEG);
  }
}

static void emit_unaop(struct cgen *c, Ast *ast, bool use_ret) {
  NodeUnaop *u = (NodeUnaop *)ast;

  gen(c, u->expr, true);

  switch(u->op) {
    case UNA_MINUS: emit_unary_neg(c, u); break;
    case UNA_NOT: push_0arg(c->iseq, OP_NOT); break;
    default: mxc_unimplemented("sorry");
  }

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_member_store(struct cgen *c, Ast *ast, bool use_ret) {
  NodeMember *m = (NodeMember *)ast;

  gen(c, m->left, true);

  NodeVariable *rhs = (NodeVariable *)m->right;

  size_t i = 0;
  for(; i < m->left->ctype->strct.nfield; ++i) {
    if(strcmp(m->left->ctype->strct.field[i]->name, rhs->name) == 0) {
      break;
    }
  }

  push32(c->iseq, OP_MEMBER_STORE, i);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_subscr_store(struct cgen *c, Ast *ast, bool use_ret) {
  NodeSubscript *l = (NodeSubscript *)ast;

  gen(c, l->index, true);
  gen(c, l->ls, true);

  push_0arg(c->iseq, OP_SUBSCR_STORE);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_assign(struct cgen *c, Ast *ast, bool use_ret) {
  // debug("called assign\n");
  NodeAssignment *a = (NodeAssignment *)ast;

  gen(c, a->src, true);

  if(a->dst->type == NDTYPE_SUBSCR) {
    emit_subscr_store(c, a->dst, use_ret);
  }
  else if(a->dst->type == NDTYPE_DOTEXPR &&
      ((NodeDotExpr *)a->dst)->t.member) {
    NodeDotExpr *dot = (NodeDotExpr *)a->dst;
    emit_member_store(c, (Ast *)dot->memb, use_ret);
  }
  else {
    emit_store(c, a->dst, use_ret);
  }
}

static void emit_funcdef(struct cgen *c, Ast *ast, bool iter) {
  NodeFunction *f = (NodeFunction *)ast;
  struct cgen *newc = newcgen(c, f->fnvar->name);

  for(int n = f->args->len - 1; n >= 0; n--) {
    NodeVariable *a = f->args->data[n];
    emit_store(newc, (Ast *)a, false);
  }

  if(f->block->type == NDTYPE_BLOCK) {
    NodeBlock *b = (NodeBlock *)f->block;
    for(size_t i = 0; i < b->cont->len; i++) {
      gen(newc, b->cont->data[i], false);
    }

    push_0arg(newc->iseq, OP_PUSHNULL);
  }
  else {
    gen(newc, f->block, true);
  }

  push_0arg(newc->iseq, OP_RET);

  userfunction *fn_object = new_userfunction(newc->iseq, newc->d);

  free(newc);

  int key = lpool_push_userfunc(c->ltable, fn_object);

  push32(c->iseq, iter? OP_ITERFN_SET : OP_FUNCTIONSET, key);

  emit_store(c, (Ast *)f->fnvar, false);
}

static void emit_if(struct cgen *c, Ast *ast) {
  NodeIf *i = (NodeIf *)ast;

  gen(c, i->cond, true);

  size_t cpos = c->iseq->len;
  push32(c->iseq, OP_JMP_NOTEQ, 0);

  gen(c, i->then_s, i->isexpr);

  if(i->else_s) {
    size_t then_epos = c->iseq->len;
    push32(c->iseq, OP_JMP, 0); // goto if statement end

    size_t else_spos = c->iseq->len;
    replace_int32(cpos, c->iseq, else_spos);

    gen(c, i->else_s, i->isexpr);

    size_t epos = c->iseq->len;
    replace_int32(then_epos, c->iseq, epos);
  }
  else {
    size_t pos = c->iseq->len;
    replace_int32(cpos, c->iseq, pos);
  }
}

static void emit_for(struct cgen *c, Ast *ast) {
  /*
   *  for i in [10, 20, 30, 40] {}
   */
  NodeFor *f = (NodeFor *)ast;

  gen(c, f->iter, true);
  push_0arg(c->iseq, OP_ITER);

  size_t loop_begin = c->iseq->len;

  size_t pos = c->iseq->len;
  push32(c->iseq, OP_ITER_NEXT, 0);

  for(int i = 0; i < f->vars->len; i++) {
    emit_store(c, f->vars->data[0], false); 
  }

  gen(c, f->body, false);

  push32(c->iseq, OP_JMP, loop_begin);

  size_t end = c->iseq->len;
  replace_int32(pos, c->iseq, end);

  if(c->loopstack->len != 0) {
    int breakp = (intptr_t)vec_pop(c->loopstack);
    replace_int32(breakp, c->iseq, end);
  }
}

static void emit_while(struct cgen *c, Ast *ast) {
  NodeWhile *w = (NodeWhile *)ast;
  size_t begin = c->iseq->len;
  gen(c, w->cond, true);

  size_t pos = c->iseq->len;
  push32(c->iseq, OP_JMP_NOTEQ, 0);
  gen(c, w->body, false);
  push32(c->iseq, OP_JMP, begin);

  size_t end = c->iseq->len;
  replace_int32(pos, c->iseq, end);

  if(c->loopstack->len != 0) {
    int breakp = (intptr_t)vec_pop(c->loopstack);
    replace_int32(breakp, c->iseq, end);
  }
}

static void emit_return(struct cgen *c, Ast *ast) {
  gen(c, ((NodeReturn *)ast)->cont, true);
  push_0arg(c->iseq, OP_RET);
}

static void emit_yield(struct cgen *c, NodeYield *y) {
  gen(c, y->cont, true);
  push_0arg(c->iseq, OP_YIELD);
}

static void emit_break(struct cgen *c, Ast *ast) {
  INTERN_UNUSE(ast);
  vec_push(c->loopstack, (void *)(intptr_t)c->iseq->len);

  push32(c->iseq, OP_JMP, 0);
}

static void emit_block(struct cgen *c, Ast *ast) {
  NodeBlock *b = (NodeBlock *)ast;

  for(int i = 0; i < b->cont->len; ++i)
    gen(c, (Ast *)b->cont->data[i], false);
}

static void emit_typed_block(struct cgen *c, Ast *ast) {
  NodeBlock *b = (NodeBlock *)ast;

  for(int i = 0; i < b->cont->len; ++i) {
    gen(c, (Ast *)b->cont->data[i],
        i == b->cont->len - 1 ? true: false);
  }
}

static void emit_vardecl(struct cgen *, Ast *);

static void emit_vardecl_block(struct cgen *c, NodeVardecl *v) {
  for(int i = 0; i < v->block->len; ++i) {
    emit_vardecl(c, v->block->data[i]);
  }
}

static void emit_vardecl(struct cgen *c, Ast *ast) {
  NodeVardecl *v = (NodeVardecl *)ast;

  if(v->is_block) {
    emit_vardecl_block(c, v);
    return;
  }

  if(v->init != NULL) {
    gen(c, v->init, true);

    emit_store(c, (Ast *)v->var, false);
  }
}

static void emit_namespace(struct cgen *c, Ast *ast) {
  NodeNameSpace *n = (NodeNameSpace *)ast;
  gen(c, (Ast *)n->block, false);
}

static void emit_load(struct cgen *c, Ast *ast, bool use_ret) {
  NodeVariable *v = (NodeVariable *)ast;
  push32(c->iseq, v->isglobal? OP_LOAD_GLOBAL : OP_LOAD_LOCAL, v->vid);

  if(!use_ret)
    push_0arg(c->iseq, OP_POP);
}

static void emit_assert(struct cgen *c, Ast *ast) {
  NodeAssert *a = (NodeAssert *)ast;
  gen(c, a->cond, true);
  push_0arg(c->iseq, OP_ASSERT);
}

static void gen(struct cgen *c, Ast *ast, bool use_ret) {
  if(ast == NULL) {
    return;
  }

  switch(ast->type) {
    case NDTYPE_NUM:
      emit_num(c, ast, use_ret);
      break;
    case NDTYPE_BOOL:
      emit_bool(c, ast, use_ret);
      break;
    case NDTYPE_NULL:
      emit_null(c, ast, use_ret);
      break;
    case NDTYPE_STRING:
      emit_string(c, ast, use_ret);
      break;
    case NDTYPE_OBJECT:
      break;
    case NDTYPE_STRUCTINIT:
      emit_struct_init(c, ast, use_ret);
      break;
    case NDTYPE_LIST:
      emit_list(c, ast, use_ret);
      break;
    case NDTYPE_HASHTABLE:
      emit_hashtable(c, ast, use_ret);
      break;
    case NDTYPE_SUBSCR:
      emit_subscr(c, ast);
      break;
    case NDTYPE_TUPLE:
      emit_tuple(c, ast);
      break;
    case NDTYPE_BINARY:
      emit_binop(c, ast, use_ret);
      break;
    case NDTYPE_MEMBER:
      emit_member(c, ast, use_ret);
      break;
    case NDTYPE_DOTEXPR:
      emit_dotexpr(c, ast, use_ret);
      break;
    case NDTYPE_UNARY:
      emit_unaop(c, ast, use_ret);
      break;
    case NDTYPE_ASSIGNMENT:
      emit_assign(c, ast, use_ret);
      break;
    case NDTYPE_IF:
    case NDTYPE_EXPRIF:
      emit_if(c, ast);
      break;
    case NDTYPE_FOR:
      emit_for(c, ast);
      break;
    case NDTYPE_WHILE:
      emit_while(c, ast);
      break;
    case NDTYPE_BLOCK:
      emit_block(c, ast);
      break;
    case NDTYPE_TYPEDBLOCK:
      emit_typed_block(c, ast);
      break;
    case NDTYPE_RETURN:
      emit_return(c, ast);
      break;
    case NDTYPE_YIELD:
      emit_yield(c, (NodeYield *)ast);
      break;
    case NDTYPE_BREAK:
      emit_break(c, ast);
      break;
    case NDTYPE_BREAKPOINT:
      push_0arg(c->iseq, OP_BREAKPOINT);
      break;
    case NDTYPE_VARIABLE:
      emit_load(c, ast, use_ret);
      break;
    case NDTYPE_FUNCCALL:
      emit_fncall(c, ast, use_ret);
      break;
    case NDTYPE_ITERATOR:
      emit_funcdef(c, ast, true);
      break;
    case NDTYPE_FUNCDEF:
      emit_funcdef(c, ast, false);
      break;
    case NDTYPE_VARDECL:
      emit_vardecl(c, ast);
      break;
    case NDTYPE_NAMESPACE:
      emit_namespace(c, ast);
      break;
    case NDTYPE_ASSERT:
      emit_assert(c, ast);
      break;
    case NDTYPE_NONENODE:
      break;
    default:
      error("??? in gen");
  }
}

struct cgen *compile(MInterp *m, Vector *ast, int ngvars) {
  struct cgen *c = newcgen_glb(ngvars);
  emit_builtins(m, c);

  for(int i = 0; i < ast->len; ++i)
    gen(c, (Ast *)ast->data[i], false);

  push_0arg(c->iseq, OP_END);

  return c;
}

struct cgen *compile_repl(MInterp *m, Vector *ast, struct cgen *p) {
  struct cgen *c = newcgen(p, "");
  emit_builtins(m, c);

  gen(c, (Ast *)ast->data[0], true);

  push_0arg(c->iseq, OP_END);
  return c;
}

