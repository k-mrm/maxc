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

env_t *Env::get() {
    return this->current;
}

bool Env::isglobal() {
    return this->current->isglb;
}

void Varlist::push(Node_variable *v) {
    this->var_v.push_back(v);
}

std::vector<Node_variable *> Varlist::get() {
    return this->var_v;
}

Node_variable *Varlist::find(std::string n) {
    for(Node_variable *v: this->var_v) {
        if(v->vinfo.name == n) return v;
    }

    error("find error: %s", n.c_str());
    return nullptr;
}

void Varlist::show() {
    printf("#varlist show: ");
    for(Node_variable *v: this->var_v) {
        std::cout << v->vinfo.name << " ";
    }
    puts("");
}

void Varlist::reset() {
    this->var_v.clear();
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
