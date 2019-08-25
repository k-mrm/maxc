#include "ast.h"
#include "error.h"

bool Ast_isexpr(Ast *self) {
    switch(self->type) {
    case NDTYPE_NUM:
    case NDTYPE_BOOL:
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
    case NDTYPE_STRUCTINIT:
    case NDTYPE_UNARY:
    case NDTYPE_TERNARY:
        return true;
    default:
        return false;
    }
}

NodeNumber *new_node_number_int(int64_t n) {
    NodeNumber *node = (NodeNumber *)malloc(sizeof(NodeNumber));

    ((Ast *)node)->type = NDTYPE_NUM;
    node->number = n;
    node->fnumber = (double)n;
    node->isfloat = false;
    ((Ast *)node)->ctype = mxcty_int;

    return node;
}

NodeNumber *new_node_number_float(double n) {
    NodeNumber *node = (NodeNumber *)malloc(sizeof(NodeNumber));

    ((Ast *)node)->type = NDTYPE_NUM;
    node->number = (int64_t)n;
    node->fnumber = n;
    node->isfloat = true;
    ((Ast *)node)->ctype = mxcty_float;

    return node;
}

NodeBool *new_node_bool(bool b) {
    NodeBool *node = malloc(sizeof(NodeBool));

    ((Ast *)node)->type = NDTYPE_BOOL;
    ((Ast *)node)->ctype = mxcty_bool;
    node->boolean = b;

    return node;
}

NodeString *new_node_string(char *s) {
    NodeString *node = malloc(sizeof(NodeString));

    ((Ast *)node)->type = NDTYPE_STRING;
    ((Ast *)node)->ctype = mxcty_string;
    node->string = s;

    return node;
}

NodeList *new_node_list(Vector *e, uint16_t n) {
    NodeList *node = malloc(sizeof(NodeList));

    ((Ast *)node)->type = NDTYPE_LIST;
    ((Ast *)node)->ctype = New_Type(CTYPE_LIST);
    node->elem = e;
    node->nsize = n;

    return node;
}

NodeTuple *new_node_tuple(Vector *e, uint16_t n, Type *ty) {
    NodeTuple *node = malloc(sizeof(NodeTuple));

    ((Ast *)node)->type = NDTYPE_TUPLE;
    node->exprs = e;
    node->nsize = n;
    ((Ast *)node)->ctype = ty;

    return node;
}

NodeBinop *new_node_binary(enum BINOP op, Ast *left, Ast *right) {
    NodeBinop *node = malloc(sizeof(NodeBinop));

    ((Ast *)node)->type = NDTYPE_BINARY;
    node->op = op;
    node->left = left;
    node->right = right;

    return node;
}

NodeMember *new_node_member(Ast *left, Ast *right) {
    NodeMember *node = malloc(sizeof(NodeMember));

    ((Ast *)node)->type = NDTYPE_MEMBER;
    node->left = left;
    node->right = right;

    return node;
}

NodeSubscript *new_node_subscript(Ast *l, Ast *i, Type *ty) {
    NodeSubscript *node = malloc(sizeof(NodeSubscript));

    ((Ast *)node)->type = NDTYPE_SUBSCR;
    node->ls = l;
    node->index = i;
    ((Ast *)node)->ctype = ty;

    return node;
}

NodeUnaop *new_node_unary(enum UNAOP op, Ast *e) {
    NodeUnaop *node = malloc(sizeof(NodeUnaop));

    ((Ast *)node)->type = NDTYPE_UNARY;
    node->op = op;
    node->expr = e;

    return node;
}

NodeFunction *new_node_function(NodeVariable *n, func_t f, Ast *b) {
    NodeFunction *node = malloc(sizeof(NodeFunction));

    ((Ast *)node)->type = NDTYPE_FUNCDEF;
    node->fnvar = n;
    node->finfo = f;
    node->block = b;
    node->lvars = New_Varlist();

    return node;
}

NodeFnCall *new_node_fncall(Ast *f, Vector *arg) {
    NodeFnCall *node = malloc(sizeof(NodeFnCall));

    ((Ast *)node)->type = NDTYPE_FUNCCALL;
    node->func = f;
    node->args = arg;

    return node;
}

NodeAssignment *new_node_assign(Ast *dst, Ast *src) {
    NodeAssignment *node = malloc(sizeof(NodeAssignment));

    ((Ast *)node)->type = NDTYPE_ASSIGNMENT;
    node->dst = dst;
    node->src = src;

    return node;
}

NodeVariable *new_node_variable(char *n) {
    NodeVariable *node = malloc(sizeof(NodeVariable));

    ((Ast *)node)->type = NDTYPE_VARIABLE;
    node->name = n;
    ((Ast *)node)->ctype = mxcty_none;

    return node;
}

NodeVariable *new_node_variable_with_var(char *n, var_t v) {
    NodeVariable *node = new_node_variable(n);

    ((Ast *)node)->ctype = v.type;
    node->vinfo = v;

    return node;
}

NodeVariable *new_node_variable_with_func(char *n, func_t f) {
    NodeVariable *node = new_node_variable(n);

    ((Ast *)node)->ctype = f.ftype;
    node->finfo = f;

    return node;
}

NodeVardecl *new_node_vardecl(NodeVariable *v, Ast *init) {
    NodeVardecl *node = malloc(sizeof(NodeVardecl));

    ((Ast *)node)->type = NDTYPE_VARDECL;
    node->var = v;
    node->init = init;

    return node;
}


NodeReturn *new_node_return(Ast *c) {
    NodeReturn *node = malloc(sizeof(NodeReturn));

    ((Ast *)node)->type = NDTYPE_RETURN;
    node->cont = c;

    return node;
}

NodeIf *new_node_if(Ast *c, Ast *t, Ast *e, bool i) {
    NodeIf *node = malloc(sizeof(NodeIf));

    ((Ast *)node)->type = i ? NDTYPE_EXPRIF : NDTYPE_IF;
    node->cond = c;
    node->then_s = t;
    node->else_s = e;
    node->isexpr = i;

    return node;
}

NodeFor *new_node_for(Ast *i, Ast *c, Ast *r, Ast *b) {
    NodeFor *node = malloc(sizeof(NodeFor));

    ((Ast *)node)->type = NDTYPE_FOR;
    node->init = i;
    node->cond = c;
    node->reinit = r;
    node->body = b;

    return node;
}

NodeWhile *new_node_while(Ast *c, Ast *b) {
    NodeWhile *node = malloc(sizeof(NodeWhile));

    ((Ast *)node)->type = NDTYPE_WHILE;
    node->cond = c;
    node->body = b;

    return node;
}

NodeStruct *new_node_struct(char *name, Vector *decls) {
    NodeStruct *node = malloc(sizeof(NodeStruct));

    ((Ast *)node)->type = NDTYPE_STRUCT;
    node->tagname = name;
    node->decls = decls;

    return node;
}

NodeStructInit *new_node_struct_init(Type *t, Vector *f, Vector *i) {
    NodeStructInit *node = malloc(sizeof(NodeStructInit));

    ((Ast *)node)->type = NDTYPE_STRUCTINIT;
    node->tag = t;
    node->fields = f;
    node->inits = i;

    return node;
}

NodeBlock *new_node_block(Vector *c) {
    NodeBlock *node = malloc(sizeof(NodeBlock));

    ((Ast *)node)->type = NDTYPE_BLOCK;
    node->cont = c;

    return node;
}
