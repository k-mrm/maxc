#include "maxc.h"

int Constant::push_var(NodeVariable *var) {
    int i = 0;
    for(auto &t: table) {
        if(t.var == nullptr) {
            ++i;
            continue;
        }
        if(var == t.var) return i;
        ++i;
    }

    int key = table.size();
    table.push_back(const_t(var));

    return key;
}

int Constant::push_str(const char *s) {
    int i = 0;
    for(auto &t: table) {
        if(t.str == nullptr) {
            ++i;
            continue;
        }
        if(strncmp(s, t.str, strlen(s)) == 0) {
            return i;
        }
        ++i;
    }

    int key = table.size();
    table.push_back(const_t(s));

    return key;
}
