#include "constant.h"
#include "maxc.h"

int Constant::push_str(const char *s) {
    int i = 0;
    for(auto &t : table) {
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

int Constant::push_userfunc(userfunction &func) {
    int key = table.size();
    table.push_back(const_t(func));

    return key;
}
