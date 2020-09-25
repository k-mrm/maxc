#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ast.h"
#include "error/error.h"

bool ast_isexpr(Ast *self) {
  if(!self) {
    return false;
  }

  switch(self->type) {
    case NDTYPE_NUM:
    case NDTYPE_BOOL:
    case NDTYPE_NULL:
    case NDTYPE_CHAR:
    case NDTYPE_LIST:
    case NDTYPE_HASHTABLE:
    case NDTYPE_SUBSCR:
    case NDTYPE_TUPLE:
    case NDTYPE_FUNCCALL:
    case NDTYPE_ASSIGNMENT:
    case NDTYPE_VARIABLE:
    case NDTYPE_STRING:
    case NDTYPE_BINARY:
    case NDTYPE_MEMBER:
    case NDTYPE_DOTEXPR:
    case NDTYPE_STRUCTINIT:
    case NDTYPE_MODULEFUNCCALL:
    case NDTYPE_UNARY:
      return true;
    default:
      return false;
  }
}

bool node_is_number(Ast *a) {
  return a && a->type == NDTYPE_NUM;
}

NodeNumber *node_number_int(int64_t n) {
  NodeNumber *node = (NodeNumber *)xmalloc(sizeof(NodeNumber));

  ((Ast *)node)->type = NDTYPE_NUM;
  node->value = mval_int(n);
  CTYPE(node) = mxcty_int;

  return node;
}

NodeNumber *node_number_big(MxcValue n) {
  NodeNumber *node = (NodeNumber *)xmalloc(sizeof(NodeNumber));

  ((Ast *)node)->type = NDTYPE_NUM;
  mgc_guard(n);
  node->value = n;
  CTYPE(node) = mxcty_int;

  return node;
}

NodeNumber *node_number_float(double n) {
  NodeNumber *node = (NodeNumber *)xmalloc(sizeof(NodeNumber));

  ((Ast *)node)->type = NDTYPE_NUM;
  node->value = mval_float(n);
  CTYPE(node) = mxcty_float;

  return node;
}

NodeChar *node_char(char c) {
  NodeChar *node = xmalloc(sizeof(NodeChar));
  ((Ast *)node)->type = NDTYPE_CHAR;
  CTYPE(node) = mxcty_char;
  node->ch = c;

  return node;
}

NodeBool *node_bool(bool b) {
  NodeBool *node = xmalloc(sizeof(NodeBool));
  ((Ast *)node)->type = NDTYPE_BOOL;
  CTYPE(node) = mxcty_bool;
  node->boolean = b;

  return node;
}

NodeNull *node_null() {
  NodeNull *node = xmalloc(sizeof(NodeNull));
  ((Ast *)node)->type = NDTYPE_NULL;
  CTYPE(node) = mxcty_any;

  return node;
}

NodeString *node_string(char *s) {
  NodeString *node = xmalloc(sizeof(NodeString));
  ((Ast *)node)->type = NDTYPE_STRING;
  CTYPE(node) = mxcty_string;
  node->string = s;

  return node;
}

NodeList *node_list(Vector *e, size_t n, Ast *nelem, Ast *init) {
  NodeList *node = xmalloc(sizeof(NodeList));
  ((Ast *)node)->type = NDTYPE_LIST;
  CTYPE(node) = new_type(CTYPE_LIST);
  node->elem = e;
  node->nsize = n;
  node->nelem = nelem;
  node->init = init;

  return node;
}

NodeHashTable *node_hashtable(Vector *k, Vector *v) {
  NodeHashTable *node = xmalloc(sizeof(NodeHashTable));
  ((Ast *)node)->type = NDTYPE_HASHTABLE;
  node->key = k;
  node->val = v;

  return node;
}

NodeTuple *node_tuple(Vector *e, uint16_t n, Type *ty) {
  NodeTuple *node = xmalloc(sizeof(NodeTuple));
  ((Ast *)node)->type = NDTYPE_TUPLE;
  node->exprs = e;
  node->nsize = n;
  CTYPE(node) = ty;

  return node;
}

NodeBinop *node_binary(enum BINOP op, Ast *left, Ast *right) {
  NodeBinop *node = xmalloc(sizeof(NodeBinop));
  ((Ast *)node)->type = NDTYPE_BINARY;
  node->op = op;
  node->left = left;
  node->right = right;
  node->impl = NULL;

  return node;
}

NodeMember *node_member(Ast *left, Ast *right) {
  NodeMember *node = xmalloc(sizeof(NodeMember));
  ((Ast *)node)->type = NDTYPE_MEMBER;
  node->left = left;
  node->right = right;

  return node;
}

NodeDotExpr *node_dotexpr(Ast *left, Ast *right) {
  NodeDotExpr *node = xmalloc(sizeof(NodeDotExpr));
  ((Ast *)node)->type = NDTYPE_DOTEXPR;
  node->left = left;
  node->right = right;
  node->t.member = node->t.fncall = 0;
  node->call = NULL;
  node->memb = NULL;

  return node;
}

NodeSubscript *node_subscript(Ast *l, Ast *i) {
  NodeSubscript *node = xmalloc(sizeof(NodeSubscript));
  ((Ast *)node)->type = NDTYPE_SUBSCR;
  node->ls = l;
  node->index = i;
  CTYPE(node) = NULL;

  return node;
}

NodeUnaop *node_unary(enum UNAOP op, Ast *e) {
  NodeUnaop *node = xmalloc(sizeof(NodeUnaop));
  ((Ast *)node)->type = NDTYPE_UNARY;
  node->op = op;
  node->expr = e;

  return node;
}

NodeFunction *node_function(NodeVariable *n,
    Ast *b,
    Vector *tyvars,
    Vector *args,
    bool iter) {
  NodeFunction *node = xmalloc(sizeof(NodeFunction));
  ((Ast *)node)->type = iter? NDTYPE_ITERATOR : NDTYPE_FUNCDEF;
  node->fnvar = n;
  node->block = b;
  node->lvars = new_vector();
  node->args = args;
  node->typevars = tyvars;
  node->is_generic = tyvars ? true : false;
  node->op = -1;

  return node;
}

NodeFnCall *node_fncall(Ast *f, Vector *arg) {
  NodeFnCall *node = xmalloc(sizeof(NodeFnCall));
  ((Ast *)node)->type = NDTYPE_FUNCCALL;
  node->func = f;
  node->args = arg;

  return node;
}

NodeAssignment *node_assign(Ast *dst, Ast *src) {
  NodeAssignment *node = xmalloc(sizeof(NodeAssignment));
  ((Ast *)node)->type = NDTYPE_ASSIGNMENT;
  node->dst = dst;
  node->src = src;

  return node;
}

NodeVariable *node_variable(char *n, int flag) {
  NodeVariable *node = xmalloc(sizeof(NodeVariable));
  ((Ast *)node)->type = NDTYPE_VARIABLE;
  node->name = n;
  node->used = false;
  node->vattr = flag;
  CTYPE(node) = mxcty_none;

  return node;
}

NodeVariable *node_variable_with_type(char *n, int flag, Type *t) {
  NodeVariable *node = node_variable(n, flag);
  CTYPE(node) = t;

  return node;
}

NodeVardecl *node_vardecl(NodeVariable *v,
    Ast *init,
    Vector *block) {
  NodeVardecl *node = xmalloc(sizeof(NodeVardecl));
  ((Ast *)node)->type = NDTYPE_VARDECL;
  node->var = v;
  node->init = init;
  node->block = block;
  node->is_block = block ? true : false;

  return node;
}

NodeReturn *node_return(Ast *c) {
  NodeReturn *node = xmalloc(sizeof(NodeReturn));
  ((Ast *)node)->type = NDTYPE_RETURN;
  node->cont = c;

  return node;
}

NodeYield *node_yield(Ast *c) {
  NodeYield *node = xmalloc(sizeof(NodeYield));
  ((Ast *)node)->type = NDTYPE_YIELD;
  node->cont = c;

  return node;
}

NodeBreak *node_break() {
  NodeBreak *node = xmalloc(sizeof(NodeBreak));
  ((Ast *)node)->type = NDTYPE_BREAK;
  node->label = 0;

  return node;
}

NodeSkip *node_skip() {
  NodeSkip *node = xmalloc(sizeof(NodeSkip));
  ((Ast *)node)->type = NDTYPE_SKIP;

  return node;
}

NodeBreakPoint *node_breakpoint() {
  NodeBreakPoint *node = xmalloc(sizeof(NodeBreakPoint));
  ((Ast *)node)->type = NDTYPE_BREAKPOINT;

  return node;
}

NodeIf *node_if(Ast *c, Ast *t, Ast *e, bool i) {
  NodeIf *node = xmalloc(sizeof(NodeIf));
  ((Ast *)node)->type = i ? NDTYPE_EXPRIF : NDTYPE_IF;
  node->cond = c;
  node->then_s = t;
  node->else_s = e;
  node->isexpr = i;

  return node;
}

NodeFor *node_for(Vector *v, Ast *i, Ast *b) {
  NodeFor *node = xmalloc(sizeof(NodeFor));
  ((Ast *)node)->type = NDTYPE_FOR;
  node->vars = v;
  node->iter = i;
  node->body = b;

  return node;
}

NodeWhile *node_while(Ast *c, Ast *b) {
  NodeWhile *node = xmalloc(sizeof(NodeWhile));
  ((Ast *)node)->type = NDTYPE_WHILE;
  node->cond = c;
  node->body = b;

  return node;
}

NodeObject *node_object(char *name, Vector *decls) {
  NodeObject *node = xmalloc(sizeof(NodeObject));
  ((Ast *)node)->type = NDTYPE_OBJECT;
  node->tagname = name;
  node->decls = decls;

  return node;
}

NodeStructInit *node_struct_init(Type *t, Vector *f, Vector *i) {
  NodeStructInit *node = xmalloc(sizeof(NodeStructInit));
  ((Ast *)node)->type = NDTYPE_STRUCTINIT;
  node->tag = t;
  node->fields = f;
  node->inits = i;

  return node;
}

NodeBlock *node_block(Vector *c) {
  NodeBlock *node = xmalloc(sizeof(NodeBlock));
  ((Ast *)node)->type = NDTYPE_BLOCK;
  node->cont = c;

  return node;
}

NodeBlock *node_typedblock(Vector *c) {
  NodeBlock *node = xmalloc(sizeof(NodeBlock));
  ((Ast *)node)->type = NDTYPE_TYPEDBLOCK;
  node->cont = c;

  return node;
}

NodeModuleFuncCall *node_modulefunccall(Ast *l, Ast *i) {
  NodeModuleFuncCall *node = xmalloc(sizeof(NodeModuleFuncCall));
  ((Ast *)node)->type = NDTYPE_MODULEFUNCCALL;
  node->name = l;
  node->ident = i;

  return node;
}

NodeNameSpace *node_namespace(char *n, NodeBlock *b) {
  NodeNameSpace *node = xmalloc(sizeof(NodeNameSpace));
  ((Ast *)node)->type = NDTYPE_NAMESPACE;
  node->name = n;
  node->block = b;

  return node;
}

NodeAssert *node_assert(Ast *a) {
  NodeAssert *node = xmalloc(sizeof(NodeAssert));
  ((Ast *)node)->type = NDTYPE_ASSERT;
  node->cond = a;

  return node;
}

NoneNode_ nonenode = {
  {
    NDTYPE_NONENODE,
    0,
    NULL,
  }
};

