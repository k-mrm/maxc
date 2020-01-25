#include "ast.h"
#include "error/error.h"

bool Ast_isexpr(Ast *self) {
    if(!self) {
        return false;
    }

    switch(self->type) {
    case NDTYPE_NUM:
    case NDTYPE_BOOL:
    case NDTYPE_NULL:
    case NDTYPE_CHAR:
    case NDTYPE_LIST:
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
    case NDTYPE_UNARY:
        return true;
    default:
        return false;
    }
}

bool node_is_number(Ast *a) {
    return a && a->type == NDTYPE_NUM;
}

NodeNumber *new_node_number_int(int64_t n) {
    NodeNumber *node = (NodeNumber *)xmalloc(sizeof(NodeNumber));

    ((Ast *)node)->type = NDTYPE_NUM;
    node->number = n;
    node->fnumber = (double)n;
    node->isfloat = false;
    ((Ast *)node)->ctype = mxcty_int;

    return node;
}

NodeNumber *new_node_number_float(double n) {
    NodeNumber *node = (NodeNumber *)xmalloc(sizeof(NodeNumber));

    ((Ast *)node)->type = NDTYPE_NUM;
    node->number = (int64_t)n;
    node->fnumber = n;
    node->isfloat = true;
    ((Ast *)node)->ctype = mxcty_float;

    return node;
}

NodeChar *new_node_char(char c) {
    NodeChar *node = xmalloc(sizeof(NodeChar));
    ((Ast *)node)->type = NDTYPE_CHAR;
    ((Ast *)node)->ctype = mxcty_char;
    node->ch = c;

    return node;
}

NodeBool *new_node_bool(bool b) {
    NodeBool *node = xmalloc(sizeof(NodeBool));
    ((Ast *)node)->type = NDTYPE_BOOL;
    ((Ast *)node)->ctype = mxcty_bool;
    node->boolean = b;

    return node;
}

NodeNull *new_node_null() {
    NodeNull *node = xmalloc(sizeof(NodeNull));
    ((Ast *)node)->type = NDTYPE_NULL;
    ((Ast *)node)->ctype = mxcty_any;

    return node;
}

NodeString *new_node_string(char *s) {
    NodeString *node = xmalloc(sizeof(NodeString));
    ((Ast *)node)->type = NDTYPE_STRING;
    ((Ast *)node)->ctype = mxcty_string;
    node->string = s;

    return node;
}

NodeList *new_node_list(Vector *e, size_t n, Ast *nelem, Ast *init) {
    NodeList *node = xmalloc(sizeof(NodeList));
    ((Ast *)node)->type = NDTYPE_LIST;
    ((Ast *)node)->ctype = New_Type(CTYPE_LIST);
    node->elem = e;
    node->nsize = n;
    node->nelem = nelem;
    node->init = init;

    return node;
}

NodeTuple *new_node_tuple(Vector *e, uint16_t n, Type *ty) {
    NodeTuple *node = xmalloc(sizeof(NodeTuple));
    ((Ast *)node)->type = NDTYPE_TUPLE;
    node->exprs = e;
    node->nsize = n;
    ((Ast *)node)->ctype = ty;

    return node;
}

NodeBinop *new_node_binary(enum BINOP op, Ast *left, Ast *right) {
    NodeBinop *node = xmalloc(sizeof(NodeBinop));
    ((Ast *)node)->type = NDTYPE_BINARY;
    node->op = op;
    node->left = left;
    node->right = right;
    node->impl = NULL;

    return node;
}

NodeMember *new_node_member(Ast *left, Ast *right) {
    NodeMember *node = xmalloc(sizeof(NodeMember));
    ((Ast *)node)->type = NDTYPE_MEMBER;
    node->left = left;
    node->right = right;

    return node;
}

NodeDotExpr *new_node_dotexpr(Ast *left, Ast *right) {
    NodeDotExpr *node = xmalloc(sizeof(NodeDotExpr));
    ((Ast *)node)->type = NDTYPE_DOTEXPR;
    node->left = left;
    node->right = right;
    node->t.member = node->t.fncall = 0;
    node->call = NULL;
    node->memb = NULL;

    return node;
}

NodeSubscript *new_node_subscript(Ast *l, Ast *i) {
    NodeSubscript *node = xmalloc(sizeof(NodeSubscript));
    ((Ast *)node)->type = NDTYPE_SUBSCR;
    node->ls = l;
    node->index = i;
    ((Ast *)node)->ctype = NULL;

    return node;
}

NodeUnaop *new_node_unary(enum UNAOP op, Ast *e) {
    NodeUnaop *node = xmalloc(sizeof(NodeUnaop));
    ((Ast *)node)->type = NDTYPE_UNARY;
    node->op = op;
    node->expr = e;

    return node;
}

NodeFunction *new_node_function(NodeVariable *n,
                                func_t f,
                                Ast *b,
                                Vector *tyvars) {
    NodeFunction *node = xmalloc(sizeof(NodeFunction));
    ((Ast *)node)->type = NDTYPE_FUNCDEF;
    node->fnvar = n;
    node->finfo = f;
    node->block = b;
    node->lvars = New_Varlist();
    node->typevars = tyvars;
    node->is_generic = tyvars ? true : false;
    node->op = -1;

    return node;
}

NodeFnCall *new_node_fncall(Ast *f, Vector *arg, Ast *fail) {
    NodeFnCall *node = xmalloc(sizeof(NodeFnCall));
    ((Ast *)node)->type = NDTYPE_FUNCCALL;
    node->func = f;
    node->args = arg;
    node->failure_block = fail;

    return node;
}

NodeAssignment *new_node_assign(Ast *dst, Ast *src) {
    NodeAssignment *node = xmalloc(sizeof(NodeAssignment));
    ((Ast *)node)->type = NDTYPE_ASSIGNMENT;
    node->dst = dst;
    node->src = src;

    return node;
}

NodeVariable *new_node_variable(char *n, int flag) {
    NodeVariable *node = xmalloc(sizeof(NodeVariable));
    ((Ast *)node)->type = NDTYPE_VARIABLE;
    node->name = n;
    node->used = false;
    node->isbuiltin = false;
    node->vattr = flag;
    ((Ast *)node)->ctype = mxcty_none;

    return node;
}

NodeVariable *new_node_variable_with_var(char *n, var_t v) {
    NodeVariable *node = new_node_variable(n, 0);
    ((Ast *)node)->ctype = v.type;
    node->vinfo = v;
    node->used = false;
    node->isbuiltin = false;

    return node;
}

NodeVariable *new_node_variable_with_func(char *n, func_t f) {
    NodeVariable *node = new_node_variable(n, 0);
    ((Ast *)node)->ctype = f.ftype;
    node->finfo = f;
    node->used = false;
    node->isbuiltin = false;

    return node;
}

NodeVardecl *new_node_vardecl(NodeVariable *v,
                              Ast *init,
                              Vector *block) {
    NodeVardecl *node = xmalloc(sizeof(NodeVardecl));
    ((Ast *)node)->type = NDTYPE_VARDECL;
    node->var = v;
    node->init = init;
    node->block = block;

    if(block) {
        node->is_block = true;
    }
    else {
        node->is_block = false;
    }

    return node;
}

NodeReturn *new_node_return(Ast *c) {
    NodeReturn *node = xmalloc(sizeof(NodeReturn));
    ((Ast *)node)->type = NDTYPE_RETURN;
    node->cont = c;

    return node;
}

NodeBreak *new_node_break() {
    NodeBreak *node = xmalloc(sizeof(NodeBreak));
    ((Ast *)node)->type = NDTYPE_BREAK;
    node->label = 0;

    return node;
}

NodeIf *new_node_if(Ast *c, Ast *t, Ast *e, bool i) {
    NodeIf *node = xmalloc(sizeof(NodeIf));
    ((Ast *)node)->type = i ? NDTYPE_EXPRIF : NDTYPE_IF;
    node->cond = c;
    node->then_s = t;
    node->else_s = e;
    node->isexpr = i;

    return node;
}

NodeFor *new_node_for(Vector *v, Ast *i, Ast *b) {
    NodeFor *node = xmalloc(sizeof(NodeFor));
    ((Ast *)node)->type = NDTYPE_FOR;
    node->vars = v;
    node->iter = i;
    node->body = b;

    return node;
}

NodeWhile *new_node_while(Ast *c, Ast *b) {
    NodeWhile *node = xmalloc(sizeof(NodeWhile));
    ((Ast *)node)->type = NDTYPE_WHILE;
    node->cond = c;
    node->body = b;

    return node;
}

NodeObject *new_node_object(char *name, Vector *decls) {
    NodeObject *node = xmalloc(sizeof(NodeObject));
    ((Ast *)node)->type = NDTYPE_OBJECT;
    node->tagname = name;
    node->decls = decls;

    return node;
}

NodeStructInit *new_node_struct_init(Type *t, Vector *f, Vector *i) {
    NodeStructInit *node = xmalloc(sizeof(NodeStructInit));
    ((Ast *)node)->type = NDTYPE_STRUCTINIT;
    node->tag = t;
    node->fields = f;
    node->inits = i;

    return node;
}

NodeBlock *new_node_block(Vector *c) {
    NodeBlock *node = xmalloc(sizeof(NodeBlock));
    ((Ast *)node)->type = NDTYPE_BLOCK;
    node->cont = c;

    return node;
}

NodeBlock *new_node_typedblock(Vector *c) {
    NodeBlock *node = xmalloc(sizeof(NodeBlock));
    ((Ast *)node)->type = NDTYPE_TYPEDBLOCK;
    node->cont = c;

    return node;
}

NodeBlock *new_node_block_nonscope(Vector *c) {
    NodeBlock *node = xmalloc(sizeof(NodeBlock));
    ((Ast *)node)->type = NDTYPE_NONSCOPE_BLOCK;
    node->cont = c;

    return node;
}

NoneNode_ *new_none_node() {
    NoneNode_ *node = xmalloc(sizeof(NoneNode_));
    ((Ast *)node)->type = NDTYPE_NONENODE;
    ((Ast *)node)->ctype = mxcty_none;

    return node;
}

