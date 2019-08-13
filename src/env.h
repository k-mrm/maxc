#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include "maxc.h"
#include "type.h"

class NodeVariable;
enum class BltinFnKind;

enum class VarAttr {
    Const = 0b0001,
    Uninit = 0b0010,
};

struct var_t {
    int vattr;
    Type *type;
};

struct arg_t {
    Type *type;
    std::string name;
};

class Varlist {
  public:
    void push(NodeVariable *v);
    void push(std::vector<NodeVariable *> &);
    std::vector<NodeVariable *> var_v;
    std::vector<NodeVariable *> &get();
    void show();
    void set_number();
    void reset();
};

// Function

struct func_t {
    BltinFnKind fnkind;
    Varlist args;
    Type *ftype;
    bool isbuiltin;

    func_t() {}
    func_t(BltinFnKind k, Type *f) : fnkind(k), ftype(f), isbuiltin(true) {}
    func_t(Type *f) : ftype(f), isbuiltin(false) {}
    func_t(Varlist a, Type *f) : args(a), ftype(f), isbuiltin(false) {}
};

struct Env {
    Varlist vars;
    Type_v userdef_type;
    Env *parent;
    bool isglb;

    Env() {}
    Env(bool i) : isglb(i) {}
};

class Scope {
  public:
    Env *current = nullptr;
    Env *make();
    Env *escape();
    bool isglobal();
};

class FuncEnv {
  public:
    Env *current = nullptr;
    Env *make();
    Env *escape();
    bool isglobal();
};

#endif
