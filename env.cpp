#include "maxc.h"

env_t *Env::make() {
    env_t *e = new env_t();
    e->parent = current;
    e->isglb = false;

    this->current = e;
    return this->current;
}

env_t *Env::escape() {
    if(!this->current->isglb) {
        this->current = this->current->parent;
        return this->current;
    }

    return nullptr;
}

env_t *Env::get() { return this->current; }

bool Env::isglobal() { return this->current->isglb; }

void Varlist::push(NodeVariable *v) { this->var_v.push_back(v); }

void Varlist::push(std::vector<NodeVariable *> &vars) {
    this->var_v.insert(var_v.end(), vars.begin(), vars.end());
}

std::vector<NodeVariable *> Varlist::get() { return this->var_v; }

NodeVariable *Varlist::find(std::string n) {
    for(NodeVariable *v : this->var_v) {
        if(v->vinfo.name == n)
            return v;
    }

    error("find error: %s", n.c_str());
    return nullptr;
}

void Varlist::show() {
    debug("varlist show: ");
    for(NodeVariable *v : this->var_v) {
        if(v->ctype->isfunction())
            std::cout << v->finfo.name << " ";
        else
            std::cout << v->vinfo.name << " ";
    }
    puts("");
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
