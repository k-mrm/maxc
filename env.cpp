#include "env.h"
#include "ast.h"
#include "error.h"

env_t *Scope::make() {
    env_t *e = new env_t();
    e->parent = current;
    e->isglb = false;

    this->current = e;
    return this->current;
}

env_t *Scope::escape() {
    if(!this->current->isglb) {
        this->current = this->current->parent;
        return this->current;
    }

    return nullptr;
}

env_t *Scope::get() { return this->current; }

bool Scope::isglobal() { return this->current->isglb; }

env_t *FuncEnv::make() {
    env_t *e = new env_t();
    e->parent = current;
    e->isglb = false;

    this->current = e;
    return this->current;
}

env_t *FuncEnv::escape() {
    if(!this->current->isglb) {
        this->current = this->current->parent;
        return this->current;
    }

    return nullptr;
}

bool FuncEnv::isglobal() { return this->current->isglb; }

void Varlist::push(NodeVariable *v) {
    v->vid = this->var_v.size();

    this->var_v.push_back(v);
}

void Varlist::push(std::vector<NodeVariable *> &vars) {
    this->var_v.insert(var_v.end(), vars.begin(), vars.end());
}

std::vector<NodeVariable *> &Varlist::get() { return var_v; }

void Varlist::show() {
    debug("varlist show: ");
    for(NodeVariable *v : this->var_v) {
        printf("%s ", v->name.c_str());
    }
    puts("");
}

void Varlist::set_number() {
    int i = 0;

    for(NodeVariable *v: this->var_v) {
        v->vid = i;
        ++i;
    }
}

void Varlist::reset() { this->var_v.clear(); }

/*
void Funclist::push(func_t f) {
    funcs.push_back(f);
}

func_t *Funclist::find(std::string n) {
    for(auto &f: funcs) {
        if(f.name == n) return &f;
    }

    return nullptr;
}
*/
