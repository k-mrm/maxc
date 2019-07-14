#ifndef MAXC_AST_H
#define MAXC_AST_H

#include "maxc.h"
#include "type.h"
#include "env.h"
#include "bltinfn.h"

enum class NDTYPE {
    NUM = 100,
    BOOL,
    CHAR,
    LIST,
    SUBSCR,
    TUPLE,
    SYMBOL,
    IDENT,
    RETURN,
    FUNCDEF,
    FUNCCALL,
    FUNCPROTO,
    VARDECL,
    ASSIGNMENT,
    VARIABLE,
    BLOCK,
    STRING,
    BINARY,
    DOT,
    UNARY,
    TERNARY,
    IF,
    EXPRIF,
    FOR,
    WHILE,
    FORMAT,
    TYPEOF,
};

class Ast {
  public:
    Type *ctype;
    virtual NDTYPE get_nd_type() = 0;
};

class NodeVariable;

typedef std::vector<Ast *> Ast_v;
typedef std::vector<NodeVariable *> Method;

class NodeNumber : public Ast {
  public:
    int number;
    double fnumber;

    bool isfloat;

    virtual NDTYPE get_nd_type() { return NDTYPE::NUM; }

    NodeNumber(int _n) : number(_n), isfloat(false) {
        ctype = new Type(CTYPE::INT);
    }
    NodeNumber(double f) : fnumber(f), isfloat(true) {
        ctype = new Type(CTYPE::DOUBLE);
    }
};

class NodeBool : public Ast {
  public:
    bool boolean;
    virtual NDTYPE get_nd_type() { return NDTYPE::BOOL; }

    NodeBool(bool b) : boolean(b) { ctype = new Type(CTYPE::BOOL); }
};

class NodeChar : public Ast {
  public:
    char ch;
    virtual NDTYPE get_nd_type() { return NDTYPE::CHAR; }

    NodeChar(char _c) : ch(_c) { ctype = new Type(CTYPE::CHAR); }
};

class NodeString : public Ast {
  public:
    std::string &string;
    virtual NDTYPE get_nd_type() { return NDTYPE::STRING; }

    Method method;

    NodeString(std::string str) : string(str) {
        ctype = new Type(CTYPE::STRING);
    }
};

class NodeList : public Ast {
  public:
    Ast_v elem;
    size_t nsize;
    Ast *nindex;

    Method method;

    virtual NDTYPE get_nd_type() { return NDTYPE::LIST; }

    NodeList(Ast_v e, size_t s) : elem(e), nsize(s) {}
    NodeList(Ast_v e, Ast *n) : elem(e), nindex(n) {
        ctype = new Type(CTYPE::LIST);
    }
};

class NodeTuple : public Ast {
  public:
    Ast_v exprs;
    size_t nsize;
    virtual NDTYPE get_nd_type() { return NDTYPE::TUPLE; }

    NodeTuple(Ast_v e, size_t n, Type *t) : exprs(e), nsize(n) { ctype = t; }
};

class NodeBinop : public Ast {
  public:
    std::string symbol;
    Ast *left;
    Ast *right;
    virtual NDTYPE get_nd_type() { return NDTYPE::BINARY; }

    NodeBinop(std::string _s, Ast *_l, Ast *_r) :
        symbol(_s), left(_l), right(_r) {}
};

class NodeDotop : public Ast {
  public:
    Ast *left;
    Ast *right;
    bool isobj = false;
    virtual NDTYPE get_nd_type() { return NDTYPE::DOT; }

    NodeDotop(Ast *l, Ast *r) : left(l), right(r) {}
};

class NodeSubscript : public Ast {
  public:
    Ast *ls;
    Ast *index;
    bool istuple = false; // default -> list
    virtual NDTYPE get_nd_type() { return NDTYPE::SUBSCR; }

    NodeSubscript(Ast *l, Ast *i, Type *t) : ls(l), index(i) { ctype = t; }
    NodeSubscript(Ast *l, Ast *i, Type *t, bool b) :
        ls(l), index(i), istuple(b) {
        ctype = t;
    }
};

class NodeUnaop : public Ast {
  public:
    std::string op;
    Ast *expr;
    virtual NDTYPE get_nd_type() { return NDTYPE::UNARY; }

    NodeUnaop(std::string _o, Ast *_e, Type *_t) : op(_o), expr(_e) {
        ctype = _t;
    }
};

class NodeTernop : public Ast {
  public:
    Ast *cond, *then, *els;
    virtual NDTYPE get_nd_type() { return NDTYPE::TERNARY; }

    NodeTernop(Ast *c, Ast *t, Ast *e) : cond(c), then(t), els(e) {}
};

class NodeAssignment : public Ast {
  public:
    Ast *dst;
    Ast *src;
    virtual NDTYPE get_nd_type() { return NDTYPE::ASSIGNMENT; }

    NodeAssignment(Ast *_d, Ast *_s) : dst(_d), src(_s) {}
};

class NodeFunction;
class NodeVariable : public Ast {
  public:
    var_t vinfo;
    func_t finfo;
    bool isglobal = false;
    size_t vid;
    virtual NDTYPE get_nd_type() { return NDTYPE::VARIABLE; }

    NodeVariable(var_t _v, bool _b) : vinfo(_v), isglobal(_b) {
        ctype = _v.type;
    }
    NodeVariable(func_t f, bool b) : finfo(f), isglobal(b) { ctype = f.ftype; }
};

class NodeVardecl : public Ast {
  public:
    NodeVariable *var;
    Ast *init;
    virtual NDTYPE get_nd_type() { return NDTYPE::VARDECL; }

    NodeVardecl(NodeVariable *_v, Ast *_i) : var(_v), init(_i) {}
};

// Node func

class NodeFunction : public Ast {
  public:
    NodeVariable *fnvar;
    func_t finfo;
    Ast_v block;
    Varlist lvars;
    virtual NDTYPE get_nd_type() { return NDTYPE::FUNCDEF; }

    NodeFunction(NodeVariable *fv, func_t f, Ast_v b, Varlist l) :
        fnvar(fv), finfo(f), block(b), lvars(l) {}
};

class NodeFnCall : public Ast {
  public:
    Ast *func;
    Ast_v args;
    virtual NDTYPE get_nd_type() { return NDTYPE::FUNCCALL; }

    NodeFnCall(Ast *f, Ast_v a) : func(f), args(a) {}
};

class NodeFnProto : public Ast {
  public:
    Type *ret_type;
    std::string name;
    Type_v types;
    virtual NDTYPE get_nd_type() { return NDTYPE::FUNCPROTO; }

    NodeFnProto(Type *_r, std::string _n, Type_v _t) :
        ret_type(_r), name(_n), types(_t) {}
};

class NodeReturn : public Ast {
  public:
    Ast *cont;
    virtual NDTYPE get_nd_type() { return NDTYPE::RETURN; }

    NodeReturn(Ast *_a) : cont(_a) {}
};

class NodeIf : public Ast {
  public:
    Ast *cond;
    Ast *then_s, *else_s;
    virtual NDTYPE get_nd_type() { return NDTYPE::IF; }

    NodeIf(Ast *_c, Ast *_t, Ast *_e) : cond(_c), then_s(_t), else_s(_e) {}
};

class NodeExprif : public Ast {
  public:
    Ast *cond;
    Ast *then_s, *else_s;
    virtual NDTYPE get_nd_type() { return NDTYPE::EXPRIF; }

    NodeExprif(Ast *_c, Ast *_t, Ast *_e) : cond(_c), then_s(_t), else_s(_e) {}
};

class NodeFor : public Ast {
  public:
    Ast *init, *cond, *reinit;
    Ast *body;
    virtual NDTYPE get_nd_type() { return NDTYPE::FOR; }

    NodeFor(Ast *_i, Ast *_c, Ast *_r, Ast *_b) :
        init(_i), cond(_c), reinit(_r), body(_b) {}
};

class NodeWhile : public Ast {
  public:
    Ast *cond;
    Ast *body;
    virtual NDTYPE get_nd_type() { return NDTYPE::WHILE; }

    NodeWhile(Ast *_c, Ast *_b) : cond(_c), body(_b) {}
};

class NodeBlock : public Ast {
  public:
    Ast_v cont;
    virtual NDTYPE get_nd_type() { return NDTYPE::BLOCK; }

    NodeBlock(Ast_v _c) : cont(_c) {}
};

class NodeFormat : public Ast {
  public:
    std::string cont;
    int narg;
    Ast_v args;
    virtual NDTYPE get_nd_type() { return NDTYPE::FORMAT; }

    NodeFormat(std::string c, int n, Ast_v a) : cont(c), narg(n), args(a) {
        ctype = new Type(CTYPE::STRING);
    }
};

class NodeTypeof : public Ast {
  public:
    NodeVariable *var;
    virtual NDTYPE get_nd_type() { return NDTYPE::TYPEOF; }

    NodeTypeof(NodeVariable *v) : var(v) {}
};

#endif
