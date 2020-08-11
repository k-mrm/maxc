#ifndef MAXC_AST_H
#define MAXC_AST_H

#include "scope.h"
#include "type.h"
#include "util.h"
#include "operator.h"
#include "object/object.h"

enum NDTYPE {
  NDTYPE_NUM = 100,
  NDTYPE_BOOL,
  NDTYPE_NULL,
  NDTYPE_CHAR,
  NDTYPE_LIST,
  NDTYPE_SUBSCR,
  NDTYPE_TUPLE,
  NDTYPE_RETURN,
  NDTYPE_YIELD,
  NDTYPE_BREAK,
  NDTYPE_SKIP,
  NDTYPE_BREAKPOINT,
  NDTYPE_FUNCDEF,
  NDTYPE_ITERATOR,
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
  NDTYPE_MODULEFUNCCALL,
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

/* AST definitions */

typedef struct NodeNumber {
  AST_HEAD;
  MxcValue value;
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
    bool member: 1;
    bool fncall: 1;
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
  Vector *args;
  Vector *lvars;
  bool is_generic;

  int op;  // operator overloading
  /* generic */
  Vector *typevars;
} NodeFunction;

struct NodeFnCall {
  AST_HEAD;
  Ast *func;
  Vector *args;
};

typedef struct NodeReturn {
  AST_HEAD;
  Ast *cont;
} NodeReturn;

typedef struct NodeYield {
  AST_HEAD;
  Ast *cont;
} NodeYield;

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

/* NodeNameSpace extends NodeVariable */
typedef struct NodeNameSpace {
  NodeVariable base;
  char *name;
  NodeBlock *block;
} NodeNameSpace;

typedef struct NodeModuleFuncCall {
  AST_HEAD;
  Ast *name;
  Ast *ident;
} NodeModuleFuncCall;

typedef struct NodeAssert {
  AST_HEAD;
  Ast *cond;
} NodeAssert;

typedef struct NoneNode_ {
  AST_HEAD;
} NoneNode_;

bool ast_isexpr(Ast *);
bool node_is_number(Ast *);

extern NoneNode_ nonenode;
#define NONE_NODE ((Ast *)&nonenode)

NodeNumber *node_number_int(int64_t);
NodeNumber *node_number_float(double);
NodeNumber *node_number_big(MxcValue);
NodeBool *node_bool(bool);
NodeNull *node_null();
NodeChar *node_char(char);
NodeString *node_string(char *);
NodeList *node_list(Vector *, size_t, Ast *, Ast *);
NodeTuple *node_tuple(Vector *, uint16_t, Type *);
NodeBinop *node_binary(enum BINOP, Ast *, Ast *);
NodeReturn *node_return(Ast *);
NodeYield *node_yield(Ast *);
NodeIf *node_if(Ast *, Ast *, Ast *, bool);
NodeFor *node_for(Vector *, Ast *, Ast *);
NodeWhile *node_while(Ast *, Ast *);
NodeMember *node_member(Ast *, Ast *);
NodeDotExpr *node_dotexpr(Ast *, Ast *);
NodeSubscript *node_subscript(Ast *, Ast *);
NodeUnaop *node_unary(enum UNAOP, Ast *);
NodeFunction *node_function(NodeVariable *, Ast *, Vector *, Vector *, bool);
NodeFnCall *node_fncall(Ast *f, Vector *, Ast *);
NodeAssignment *node_assign(Ast *, Ast *);
NodeVardecl *node_vardecl(NodeVariable *, Ast *, Vector *);
NodeVariable *node_variable(char *, int);
NodeVariable *node_variable_with_type(char *, int, Type *);
NodeObject *node_object(char *, Vector *);
NodeStructInit *node_struct_init(Type *, Vector *, Vector *);
NodeBlock *node_block(Vector *);
NodeBreak *node_break(void);
NodeSkip *node_skip(void);
NodeBreakPoint *node_breakpoint(void);
NodeBlock *node_typedblock(Vector *);
NodeNameSpace *node_namespace(char *, NodeBlock *);
NodeModuleFuncCall *node_modulefunccall(Ast *, Ast *);
NodeAssert *node_assert(Ast *);

#define CAST_AST(node) ((Ast *)(node))
#define CAST_TYPE(ty) ((Type *)(ty))

#endif
