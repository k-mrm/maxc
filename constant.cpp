#include "constant.h"
#include "maxc.h"

int Constant::push_str(std::string &s) {
    int i = 0;
    for(auto &t : table) {
        if(t.str == s) {
            return i;
        }
        ++i;
    }

    int key = table.size();
    table.push_back(const_t(s));

    return key;
}

int Constant::push_float(double fnum) {
    int i = 0;
    for(auto &t : table) {
        if(t.number == fnum) {
            return i;
        }

        ++i;
    }

    int key = table.size();
    table.push_back(const_t(fnum));

    return key;
}

int Constant::push_userfunc(userfunction &func) {
    int key = table.size();
    table.push_back(const_t(func));

    return key;
}
