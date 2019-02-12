#include "maxc.h"

env_t *Env::make() {
    env_t *e = new env_t();
    e->parent = current;
    e->isglb = false;

    current = e;
    return current;
}

env_t *Env::escape() {
    if(!current->isglb) {
        current = current->parent;
        return current;
    }

    return nullptr;
}

env_t *Env::get_cur() {
    return current;
}

bool Env::isglobal() {
    return current->isglb;
}

void Varlist::push(Node_variable *v) {
    var_v.push_back(v);
}

Node_variable *Varlist::find(std::string n) {
    if(var_v.empty())
        return nullptr;
    for(Node_variable *v: var_v) {
        if(v->vinfo.name == n) return v;
    }

    return nullptr;
}

void Varlist::reset() {
    var_v.clear();
}

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
