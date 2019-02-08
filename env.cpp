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

void Variables::push(var_t v) {
    var_v.push_back(v);
}
