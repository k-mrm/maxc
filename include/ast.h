#ifndef MAXC_AST_H
#define MAXC_AST_H

#include "env.h"
#include "type.h"
#include "util.h"
#include "operator.h"
#include "builtins.h"

enum NDTYPE {
    NDTYPE_NUM = 100,
    NDTYPE_BOOL,
    NDTYPE_NULL,
    NDTYPE_CHAR,
    NDTYPE_LIST,
    NDTYPE_SUBSCR,
    NDTYPE_TUPLE,
    NDTYPE_RETURN,
    NDTYPE_BREAK,
    NDTYPE_SKIP,
    NDTYPE_BREAKPOINT,
    NDTYPE_FUNCDEF,
    NDTYPE_FUNCCALL,
    NDTYPE_FUNCPROTO,
    NDTYPE_VARDECL,
    NDTYPE_ASSIGNMENT,
    NDTYPE_VARIABLE,
    NDTYPE_OBJECT,
    NDTYPE_STRUCTINIT,
    NDTYPE_MODULE,
    NDTYPE_BLOCK,
    NDTYPE_TYPEDBLOCK,
    NDTYPE_STRING,
    NDTYPE_BINARY,
    NDTYPE_MEMBER,
    NDTYPE_DOTEXPR,
    NDTYPE_UNARY,
    NDTYPE_TERNARY,
    NDTYPE_IF,
    NDTYPE_EXPRIF,
    NDTYPE_FOR,
    NDTYPE_WHILE,
    NDTYPE_NAMESOLVER,
    NDTYPE_NAMESPACE,
    NDTYPE_ASSERT,
    NDTYPE_NONENODE,
};

typedef struct Ast {
    enum NDTYPE type;
    int line;
    Type *ctype;
} Ast;

#define AST_HEAD Ast base

#define CTYPE(ast) (((Ast *)(ast))->ctype)

struct NodeVariable;
typedef struct NodeFnCall NodeFnCall;

/* Ast definitions */

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

typedef struct NodeNull {
    AST_HEAD;
} NodeNull;

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
    /* let a = [10, 20, 30, 40]; */
    Vector *elem;
    size_t nsize;
    /* let a = [200; 0]; */
    Ast *nelem;
    Ast *init;
} NodeList;

typedef struct NodeTuple {
    AST_HEAD;
    Vector *exprs;
    size_t nsize;
} NodeTuple;

typedef struct NodeBinop {
    AST_HEAD;
    enum BINOP op;
    Ast *left;
    Ast *right;

    struct NodeFnCall *impl;
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

typedef struct NodeUnaop {
    AST_HEAD;
    enum UNAOP op;
    Ast *expr;
} NodeUnaop;

typedef struct NodeDotExpr {
    AST_HEAD;
    Ast *left;
    Ast *right;
    struct {
        unsigned int member: 1;
        unsigned int fncall: 1;
    } t;

    NodeFnCall *call;
    NodeMember *memb;
} NodeDotExpr;

typedef struct NodeAssignment {
    AST_HEAD;
    Ast *dst;
    Ast *src;
} NodeAssignment;

typedef struct NodeVariable {
    AST_HEAD;
    char *name;
    bool isglobal;
    size_t vid;
    int vattr;
    bool used;  // for warning
    /* built-in function */
    bool isbuiltin;
    /* for overload */
    bool is_overload;
    NodeVariable *next;
} NodeVariable;

typedef struct NodeVardecl {
    AST_HEAD;
    NodeVariable *var;
    Ast *init;

    /* decl block */
    bool is_block;
    Vector *block;
} NodeVardecl;

typedef struct NodeObject {
    AST_HEAD;
    char *tagname;
    Vector *decls;
} NodeObject;

typedef struct NodeStructInit {
    AST_HEAD;
    Type *tag;
    Vector *fields;
    Vector *inits;
} NodeStructInit;

/* Node func */
struct NodeBlock;

typedef struct NodeFunction {
    AST_HEAD;
    NodeVariable *fnvar;
    Ast *block;
    Varlist *args;
    Varlist *lvars;
    bool is_generic;

    int op;  // operator overloading
    /* generic */
    Vector *typevars;
} NodeFunction;

struct NodeFnCall {
    AST_HEAD;
    Ast *func;
    Vector *args;

    Ast *failure_block;
};

typedef struct NodeReturn {
    AST_HEAD;
    Ast *cont;
} NodeReturn;

typedef struct NodeBreak {
    AST_HEAD;
    int label;
} NodeBreak;

typedef struct NodeSkip {
    AST_HEAD;
} NodeSkip;

typedef struct NodeBreakPoint {
    AST_HEAD;
} NodeBreakPoint;

typedef struct NodeIf {
    AST_HEAD;
    Ast *cond;
    Ast *then_s, *else_s;
    bool isexpr;
} NodeIf;

typedef struct NodeFor {
    AST_HEAD;
    Vector *vars;
    Ast *iter;
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

typedef struct NodeNameSpace {
    AST_HEAD;
    char *name;
    NodeBlock *block;
} NodeNameSpace;

typedef struct NodeNameSolver {
    AST_HEAD;
    Ast *name;
    Ast *ident;
} NodeNameSolver;

typedef struct NodeAssert {
    AST_HEAD;
    Ast *cond;
} NodeAssert;

typedef struct NoneNode_ {
    AST_HEAD;
} NoneNode_;

bool Ast_isexpr(Ast *);
bool node_is_number(Ast *);

extern NoneNode_ nonenode;
#define NONE_NODE ((Ast *)&nonenode)

NodeNumber *new_node_number_int(int64_t);
NodeNumber *new_node_number_float(double);
NodeBool *new_node_bool(bool);
NodeNull *new_node_null();
NodeChar *new_node_char(char);
NodeString *new_node_string(char *);
NodeList *new_node_list(Vector *, size_t, Ast *, Ast *);
NodeTuple *new_node_tuple(Vector *, uint16_t, Type *);
NodeBinop *new_node_binary(enum BINOP, Ast *, Ast *);
NodeReturn *new_node_return(Ast *);
NodeIf *new_node_if(Ast *, Ast *, Ast *, bool);
NodeFor *new_node_for(Vector *, Ast *, Ast *);
NodeWhile *new_node_while(Ast *, Ast *);
NodeMember *new_node_member(Ast *, Ast *);
NodeDotExpr *new_node_dotexpr(Ast *, Ast *);
NodeSubscript *new_node_subscript(Ast *, Ast *);
NodeUnaop *new_node_unary(enum UNAOP, Ast *);
NodeFunction *new_node_function(NodeVariable *, Ast *, Vector *, Varlist *);
NodeFnCall *new_node_fncall(Ast *f, Vector *, Ast *);
NodeAssignment *new_node_assign(Ast *, Ast *);
NodeVardecl *new_node_vardecl(NodeVariable *, Ast *, Vector *);
NodeVariable *new_node_variable(char *, int);
NodeVariable *new_node_variable_with_type(char *, int, Type *);
NodeObject *new_node_object(char *, Vector *);
NodeStructInit *new_node_struct_init(Type *, Vector *, Vector *);
NodeBlock *new_node_block(Vector *);
NodeBreak *new_node_break(void);
NodeSkip *new_node_skip(void);
NodeBreakPoint *new_node_breakpoint(void);
NodeBlock *new_node_typedblock(Vector *);
NodeNameSpace *new_node_namespace(char *, NodeBlock *);
NodeNameSolver *new_node_namesolver(Ast *, Ast *);
NodeAssert *new_node_assert(Ast *);

#define CAST_AST(node) ((Ast *)(node))
#define CAST_TYPE(node) ((Type *)(node))

#endif
