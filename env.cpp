#include "maxc.h"

env_t *Env::make() {
    env_t *e = new env_t;
    e->parent = current;
    if(current)
        current->child.push_back(e);

    current = e;
    return current;
}

env_t *Env::escape() {
    if(current->parent) {
        current = current->parent;
        return current;
    }

    return nullptr;
}

void Varlist::push(var_t v) {
    var_v.push_back(v);
}

void Funclist::push(func_t f) {
    funcs.push_back(f);
}

func_t *Funclist::find(std::string n) {
    for(auto &f: funcs) {
        if(f.name == n) {
            return &f;
        }
    }

    return nullptr;
}
