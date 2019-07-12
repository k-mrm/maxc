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
    std::string name;
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
    std::vector<NodeVariable *> get();
    void show();
    void reset();
};

// Function

struct func_t {
    std::string name;
    BltinFnKind fnkind;
    Varlist args;
    Type *ftype;
    bool isbuiltin;

    func_t() {}
    func_t(std::string n, BltinFnKind k, Type *f) :
        name(n), fnkind(k), ftype(f), isbuiltin(true) {}
    func_t(std::string n, Type *f) : name(n), ftype(f), isbuiltin(false) {}
    func_t(std::string n, Varlist a, Type *f) :
        name(n), args(a), ftype(f), isbuiltin(false) {}
};

struct env_t {
    Varlist vars;
    env_t *parent;
    bool isglb;

    env_t() {}
    env_t(bool i) : isglb(i) {}
};

class Env {
  public:
    env_t *current = nullptr;
    env_t *make();
    env_t *escape();
    env_t *get();
    bool isglobal();
};

#endif
