#ifndef MAXC_AST_H
#define MAXC_AST_H

#include "env.h"
#include "function.h"
#include "maxc.h"
#include "method.h"
#include "type.h"
#include "util.h"

enum NDTYPE {
    NDTYPE_NUM = 100,
    NDTYPE_BOOL,
    NDTYPE_CHAR,
    NDTYPE_LIST,
    NDTYPE_SUBSCR,
    NDTYPE_TUPLE,
    NDTYPE_RETURN,
    NDTYPE_BREAK,
    NDTYPE_FUNCDEF,
    NDTYPE_FUNCCALL,
    NDTYPE_FUNCPROTO,
    NDTYPE_VARDECL,
    NDTYPE_ASSIGNMENT,
    NDTYPE_VARIABLE,
    NDTYPE_STRUCT,
    NDTYPE_STRUCTINIT,
    NDTYPE_MODULE,
    NDTYPE_BLOCK,
    NDTYPE_TYPEDBLOCK,
    NDTYPE_NONSCOPE_BLOCK,
    NDTYPE_STRING,
    NDTYPE_BINARY,
    NDTYPE_MEMBER,
    NDTYPE_UNARY,
    NDTYPE_TERNARY,
    NDTYPE_IF,
    NDTYPE_EXPRIF,
    NDTYPE_FOR,
    NDTYPE_WHILE,
};

typedef struct Ast {
    enum NDTYPE type;
    Type *ctype;
} Ast;

#define AST_HEAD Ast base

struct NodeVariable;

typedef struct NodeNumber {
    AST_HEAD;
    int64_t number;
    double fnumber;

    bool isfloat;
} NodeNumber;

typedef struct NodeBool {
    AST_HEAD;
    bool boolean;
} NodeBool;

typedef struct NodeChar {
    AST_HEAD;
    char ch;
} NodeChar;

typedef struct NodeString {
    AST_HEAD;
    char *string;
} NodeString;

typedef struct NodeList {
    AST_HEAD;
    Vector *elem;
    size_t nsize;
    Ast *nindex;
} NodeList;

typedef struct NodeTuple {
    AST_HEAD;
    Vector *exprs;
    size_t nsize;
} NodeTuple;

enum BINOP {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,
    BIN_EQ,
    BIN_NEQ,
    BIN_LT,
    BIN_LTE,
    BIN_GT,
    BIN_GTE,
    BIN_LAND,
    BIN_LOR
};

typedef struct NodeBinop {
    AST_HEAD;
    enum BINOP op;
    Ast *left;
    Ast *right;
} NodeBinop;

typedef struct NodeMember {
    AST_HEAD;
    Ast *left;
    Ast *right;
} NodeMember;

typedef struct NodeSubscript {
    AST_HEAD;
    Ast *ls;
    Ast *index;
    bool istuple; // default: list
} NodeSubscript;

enum UNAOP {
    UNA_INC,
    UNA_DEC,
    UNA_PLUS,
    UNA_MINUS,
};

typedef struct NodeUnaop {
    AST_HEAD;
    enum UNAOP op;
    Ast *expr;
} NodeUnaop;

typedef struct NodeAssignment {
    AST_HEAD;
    Ast *dst;
    Ast *src;
} NodeAssignment;

typedef struct NodeVariable {
    AST_HEAD;
    char *name;
    var_t vinfo;
    func_t finfo;
    bool isglobal;
    size_t vid;

    bool used;  // for warning
    bool isbuiltin;
} NodeVariable;

typedef struct NodeVardecl {
    AST_HEAD;
    NodeVariable *var;
    Ast *init;
} NodeVardecl;

typedef struct NodeStruct {
    AST_HEAD;
    char *tagname;
    Vector *decls;
} NodeStruct;

typedef struct NodeStructInit {
    AST_HEAD;
    Type *tag;
    Vector *fields;
    Vector *inits;
} NodeStructInit;

// Node func
struct NodeBlock;

typedef struct NodeFunction {
    AST_HEAD;
    NodeVariable *fnvar;
    func_t finfo;
    Ast *block;
    Varlist *lvars;
} NodeFunction;

typedef struct NodeFnCall {
    AST_HEAD;
    Ast *func;
    Vector *args;

    Ast *failure_block;
} NodeFnCall;

typedef struct NodeReturn {
    AST_HEAD;
    Ast *cont;
} NodeReturn;

typedef struct NodeBreak {
    AST_HEAD;
} NodeBreak;

typedef struct NodeIf {
    AST_HEAD;
    Ast *cond;
    Ast *then_s, *else_s;
    bool isexpr;
} NodeIf;

typedef struct NodeFor {
    AST_HEAD;
    Ast *init, *cond, *reinit;
    Ast *body;
} NodeFor;

typedef struct NodeWhile {
    AST_HEAD;
    Ast *cond;
    Ast *body;
} NodeWhile;

typedef struct NodeBlock {
    AST_HEAD;
    Vector *cont;
} NodeBlock;

bool Ast_isexpr(Ast *self);
NodeNumber *new_node_number_int(int64_t);
NodeNumber *new_node_number_float(double);
NodeBool *new_node_bool(bool);
NodeString *new_node_string(char *);
NodeList *new_node_list(Vector *, uint16_t);
NodeTuple *new_node_tuple(Vector *, uint16_t, Type *);
NodeBinop *new_node_binary(enum BINOP, Ast *, Ast *);
NodeReturn *new_node_return(Ast *);
NodeIf *new_node_if(Ast *, Ast *, Ast *, bool);
NodeFor *new_node_for(Ast *, Ast *, Ast *, Ast *);
NodeWhile *new_node_while(Ast *, Ast *);
NodeMember *new_node_member(Ast *, Ast *);
NodeSubscript *new_node_subscript(Ast *, Ast *, Type *);
NodeUnaop *new_node_unary(enum UNAOP, Ast *);
NodeFunction *new_node_function(NodeVariable *, func_t, Ast *);
NodeFnCall *new_node_fncall(Ast *f, Vector *, Ast *);
NodeAssignment *new_node_assign(Ast *, Ast *);
NodeVardecl *new_node_vardecl(NodeVariable *, Ast *);
NodeVariable *new_node_variable(char *);
NodeVariable *new_node_variable_with_var(char *, var_t);
NodeVariable *new_node_variable_with_func(char *, func_t);
NodeStruct *new_node_struct(char *, Vector *);
NodeStructInit *new_node_struct_init(Type *, Vector *, Vector *);
NodeBlock *new_node_block(Vector *);
NodeBreak *new_node_break();
NodeBlock *new_node_typedblock(Vector *);
NodeBlock *new_node_block_nonscope(Vector *);

#define CAST_AST(node) ((Ast *)(node))
#define CAST_TYPE(node) ((Type *)(node))

#endif
