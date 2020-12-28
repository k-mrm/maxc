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

extern const char *filename;

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
  vec_push(c->d->pc_line_map, (void *)(intptr_t)lineno);
  pushop(c->iseq, a);
}

void cpush8(struct cgen *c, enum OPCODE op, uint8_t a, int lineno) {
  vec_push(c->d->pc_line_map, (void *)(intptr_t)lineno);
  vec_push(c->d->pc_line_map, (void *)(intptr_t)lineno);
  push8(c->iseq, op, a);
}

void cpush32(struct cgen *c, enum OPCODE op, uint32_t a, int lineno) {
  vec_push(c->d->pc_line_map, (void *)(intptr_t)lineno);
  vec_push(c->d->pc_line_map, (void *)(intptr_t)lineno);
  push32(c->iseq, op, a);
}

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

static void emit_builtins(Vector *module, struct cgen *c) {
  for(size_t i = 0; i < module->len; ++i) {
    MxcModule *mod = (MxcModule *)module->data[i];
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
    int key = lpool_push_float(c->ltable, V2F(n->value));
    cpush32(c, OP_FPUSH, key, lno);
  }
  else if(isobj(n->value)) {
    emit_rawobject(c, n->value, true);
  }
  else {
    switch(V2I(n->value)) {
      case 0:     cpush(c, OP_PUSHCONST_0, lno); break;
      case 1:     cpush(c, OP_PUSHCONST_1, lno); break;
      case 2:     cpush(c, OP_PUSHCONST_2, lno); break;
      case 3:     cpush(c, OP_PUSHCONST_3, lno); break;
      default:    cpush32(c, OP_IPUSH, V2I(n->value), lno);  break;
    }
  }

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_bool(struct cgen *c, Ast *ast, bool use_ret) {
  NodeBool *b = (NodeBool *)ast;
  int lno = ast->lineno;
  cpush(c, (b->boolean ? OP_PUSHTRUE : OP_PUSHFALSE), lno);

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
    cpush(c, OP_POP, lno);
}

static void emit_struct_init(struct cgen *c, Ast *ast, bool use_ret) {
  NodeStructInit *s = (NodeStructInit *)ast;
  int lno = LINENO(ast);

  push32(c->iseq, OP_STRUCTSET, CAST_AST(s)->ctype->strct.nfield);

  // TODO

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_subscr(struct cgen *c, Ast *ast) {
  NodeSubscript *l = (NodeSubscript *)ast;
  int lno = LINENO(l);

  gen(c, l->index, true);
  gen(c, l->ls, true);

  cpush(c, OP_SUBSCR, lno);
}

static void emit_tuple(struct cgen *c, Ast *ast) {
  NodeTuple *t = (NodeTuple *)ast;

  for(int i = (int)t->nsize - 1; i >= 0; i--)
    gen(c, (Ast *)t->exprs->data[i], true);
}

static void emit_catch(struct cgen *c, NodeBinop *b, bool use_ret) {
  int lno = LINENO(b);
  cpush(c, OP_TRY, lno);

  gen(c, b->left, true);
  size_t catch_pos = c->iseq->len;
  cpush32(c, OP_CATCH, 0, lno);
  gen(c, b->right, true);

  replace_int32(catch_pos, c->iseq, c->iseq->len);
  
  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_binop(struct cgen *c, Ast *ast, bool use_ret) {
  NodeBinop *b = (NodeBinop *)ast;
  int lno = LINENO(b);

  if(b->op == BIN_QUESTION)
    return emit_catch(c, b, use_ret);

  gen(c, b->left, true);
  gen(c, b->right, true);

  if(type_is(b->left->ctype, CTYPE_INT)) {
    switch(b->op) {
      case BIN_ADD:   cpush(c, OP_ADD, lno); break;
      case BIN_SUB:   cpush(c, OP_SUB, lno); break;
      case BIN_MUL:   cpush(c, OP_MUL, lno); break;
      case BIN_DIV:   cpush(c, OP_DIV, lno); break;
      case BIN_MOD:   cpush(c, OP_MOD, lno); break;
      case BIN_EQ:    cpush(c, OP_EQ, lno); break;
      case BIN_NEQ:   cpush(c, OP_NOTEQ, lno); break;
      case BIN_LOR:   cpush(c, OP_LOGOR, lno); break;
      case BIN_LAND:  cpush(c, OP_LOGAND, lno); break;
      case BIN_LT:    cpush(c, OP_LT, lno); break;
      case BIN_LTE:   cpush(c, OP_LTE, lno); break;
      case BIN_GT:    cpush(c, OP_GT, lno); break;
      case BIN_GTE:   cpush(c, OP_GTE, lno); break;
      case BIN_BXOR:  cpush(c, OP_BXOR, lno); break;
      default:        break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_FLOAT)){
    switch(b->op) {
      case BIN_ADD:   cpush(c, OP_FADD, lno); break;
      case BIN_SUB:   cpush(c, OP_FSUB, lno); break;
      case BIN_MUL:   cpush(c, OP_FMUL, lno); break;
      case BIN_DIV:   cpush(c, OP_FDIV, lno); break;
      case BIN_MOD:   cpush(c, OP_FMOD, lno); break;
      case BIN_EQ:    cpush(c, OP_FEQ, lno); break;
      case BIN_NEQ:   cpush(c, OP_FNOTEQ, lno); break;
      case BIN_LOR:   cpush(c, OP_FLOGOR, lno); break;
      case BIN_LAND:  cpush(c, OP_FLOGAND, lno); break;
      case BIN_LT:    cpush(c, OP_FLT, lno); break;
      case BIN_LTE:   cpush(c, OP_FLTE, lno); break;
      case BIN_GT:    cpush(c, OP_FGT, lno); break;
      case BIN_GTE:   cpush(c, OP_FGTE, lno); break;
      default:        break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_STRING)){
    switch(b->op) {
      case BIN_ADD:   cpush(c, OP_STRCAT, lno); break;
      case BIN_EQ: {
        emit_rawobject(c, new_cfunc(mstr_eq), true);
        cpush32(c, OP_CALL, 2, lno);
      }
      default: break;
    }
  }
  else if(type_is(b->left->ctype, CTYPE_BOOL)) {
    switch(b->op) {
      case BIN_LOR:   cpush(c, OP_LOGOR, lno); break;
      case BIN_LAND:  cpush(c, OP_LOGAND, lno); break;
      case BIN_EQ:    cpush(c, OP_EQ, lno); break;
      case BIN_NEQ:   cpush(c, OP_NOTEQ, lno); break;
      default:        break;
    }
  }

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_member(struct cgen *c, Ast *ast, bool use_ret) {
  NodeMember *m = (NodeMember *)ast;
  int lno = LINENO(m);

  gen(c, m->left, true);
  NodeVariable *rhs = (NodeVariable *)m->right;

  size_t i = 0;
  for(; i < m->left->ctype->strct.nfield; ++i) {
    if(strcmp(m->left->ctype->strct.field[i]->name, rhs->name) == 0) {
      break;
    }
  }
  cpush32(c, OP_MEMBER_LOAD, i, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_fncall(struct cgen *c, Ast *ast, bool use_ret) {
  NodeFnCall *f = (NodeFnCall *)ast;
  int lno = LINENO(f);

  for(int i = 0; i < f->args->len; i++)
    gen(c, (Ast *)f->args->data[i], true);
  gen(c, f->func, true);

  cpush32(c, OP_CALL, f->args->len, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_dotexpr(struct cgen *c, Ast *ast, bool use_ret) {
  NodeDotExpr *d = (NodeDotExpr *)ast;

  if(d->t.member)
    emit_member(c, (Ast *)d->memb, use_ret);
  else if(d->t.fncall)
    emit_fncall(c, (Ast *)d->call, use_ret);
}

static void emit_unary_neg(struct cgen *c, NodeUnaop *u) {
  int lno = LINENO(u);
  if(type_is(((Ast *)u)->ctype, CTYPE_INT)) {
    cpush(c, OP_INEG, lno);
  }
  else if(type_is(((Ast *)u)->ctype, CTYPE_FLOAT)) {
    cpush(c, OP_FNEG, lno);
  }
}

static void emit_unaop(struct cgen *c, Ast *ast, bool use_ret) {
  NodeUnaop *u = (NodeUnaop *)ast;
  int lno = LINENO(u);

  gen(c, u->expr, true);

  switch(u->op) {
    case UNA_MINUS: emit_unary_neg(c, u); break;
    case UNA_NOT:   cpush(c, OP_NOT, lno); break;
    default:        mxc_unimplemented("sorry");
  }

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_member_store(struct cgen *c, Ast *ast, bool use_ret) {
  NodeMember *m = (NodeMember *)ast;
  int lno = LINENO(m);

  gen(c, m->left, true);

  NodeVariable *rhs = (NodeVariable *)m->right;

  size_t i = 0;
  for(; i < m->left->ctype->strct.nfield; ++i) {
    if(strcmp(m->left->ctype->strct.field[i]->name, rhs->name) == 0) {
      break;
    }
  }

  cpush32(c, OP_MEMBER_STORE, i, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_subscr_store(struct cgen *c, Ast *ast, bool use_ret) {
  NodeSubscript *l = (NodeSubscript *)ast;
  int lno = LINENO(l);

  gen(c, l->index, true);
  gen(c, l->ls, true);

  cpush(c, OP_SUBSCR_STORE, lno);

  if(!use_ret)
    cpush(c, OP_POP, lno);
}

static void emit_assign(struct cgen *c, Ast *ast, bool use_ret) {
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
  int lno = LINENO(f);
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

    cpush(newc, OP_PUSHNULL, lno);
  }
  else {
    gen(newc, f->block, true);
  }

  cpush(newc, OP_RET, lno);

  userfunction *fn_object = new_userfunction(newc->iseq, newc->d, f->lvars->len);

  free(newc);

  int key = lpool_push_userfunc(c->ltable, fn_object);

  cpush32(c, iter? OP_ITERFN_SET : OP_FUNCTIONSET, key, lno);

  emit_store(c, (Ast *)f->fnvar, false);
}

static void emit_if(struct cgen *c, Ast *ast) {
  NodeIf *i = (NodeIf *)ast;
  int lno = LINENO(i);

  gen(c, i->cond, true);

  size_t cpos = c->iseq->len;
  cpush32(c, OP_JMP_NOTEQ, 0, lno);

  gen(c, i->then_s, i->isexpr);

  if(i->else_s) {
    size_t then_epos = c->iseq->len;
    cpush32(c, OP_JMP, 0, lno); // goto if statement end

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
  int lno = LINENO(f);

  gen(c, f->iter, true);
  cpush(c, OP_ITER, lno);

  size_t loop_begin = c->iseq->len;

  size_t pos = c->iseq->len;
  cpush32(c, OP_ITER_NEXT, 0, lno);

  for(int i = 0; i < f->vars->len; i++) {
    emit_store(c, f->vars->data[0], false); 
  }

  gen(c, f->body, false);

  cpush32(c, OP_JMP, loop_begin, lno);

  size_t end = c->iseq->len;
  replace_int32(pos, c->iseq, end);

  if(c->loopstack->len != 0) {
    int breakp = (intptr_t)vec_pop(c->loopstack);
    replace_int32(breakp, c->iseq, end);
  }
}

static void emit_while(struct cgen *c, Ast *ast) {
  NodeWhile *w = (NodeWhile *)ast;
  int lno = LINENO(w);
  size_t begin = c->iseq->len;
  gen(c, w->cond, true);

  size_t pos = c->iseq->len;
  cpush32(c, OP_JMP_NOTEQ, 0, lno);
  gen(c, w->body, false);
  cpush32(c, OP_JMP, begin, lno);

  size_t end = c->iseq->len;
  replace_int32(pos, c->iseq, end);

  if(c->loopstack->len != 0) {
    int breakp = (intptr_t)vec_pop(c->loopstack);
    replace_int32(breakp, c->iseq, end);
  }
}

static void emit_switch(struct cgen *c, Ast *ast) {
  NodeSwitch *s = (NodeSwitch *)ast;
  int lno = LINENO(s);

  gen(c, s->match, true);
  size_t dis_pos = c->iseq->len;
  cpush32(c, OP_SWITCH_DISPATCH, 0, lno);

  for(int i = 0; i < s->ecase->len; i++) {
    Ast *ec = s->ecase->data[i];
    Ast *b = s->body->data[i];
    gen(c, ec, true);
    gen(c, b, false);
  }

  size_t elsepos = c->iseq->len;
  replace_int32(dis_pos, c->iseq, elsepos);
  gen(c, s->eelse, false);
}

static void emit_return(struct cgen *c, Ast *ast) {
  gen(c, ((NodeReturn *)ast)->cont, true);
  cpush(c, OP_RET, LINENO(ast));
}

static void emit_yield(struct cgen *c, NodeYield *y) {
  gen(c, y->cont, true);
  cpush(c, OP_YIELD, LINENO(y));
}

static void emit_break(struct cgen *c, Ast *ast) {
  INTERN_UNUSE(ast);
  vec_push(c->loopstack, (void *)(intptr_t)c->iseq->len);

  cpush32(c, OP_JMP, 0, LINENO(ast));
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
  cpush32(c, v->isglobal? OP_LOAD_GLOBAL : OP_LOAD_LOCAL, v->vid, LINENO(ast));

  if(!use_ret)
    cpush(c, OP_POP, LINENO(ast));
}

static void emit_assert(struct cgen *c, Ast *ast) {
  NodeAssert *a = (NodeAssert *)ast;
  gen(c, a->cond, true);
  cpush(c, OP_ASSERT, LINENO(ast));
}

static void gen(struct cgen *c, Ast *ast, bool use_ret) {
  if(!ast)
    return;

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
    case NDTYPE_SWITCH:
      emit_switch(c, ast);
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

struct cgen *compile(Vector *m, Vector *ast, int ngvars) {
  struct cgen *c = newcgen_glb(ngvars);
  emit_builtins(m, c);

  for(int i = 0; i < ast->len; ++i)
    gen(c, (Ast *)ast->data[i], false);

  cpush(c, OP_END, -1);

  return c;
}

struct cgen *compile_repl(Vector *m, Vector *ast, struct cgen *p) {
  struct cgen *c = newcgen(p, "");
  emit_builtins(m, c);

  gen(c, (Ast *)ast->data[0], true);

  cpush(c, OP_END, -1);
  return c;
}

