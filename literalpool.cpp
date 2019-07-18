#include "literalpool.h"
#include "maxc.h"

int LiteralPool::push_str(std::string &s) {
    int i = 0;
    for(auto &t : table) {
        if(t.str == s) {
            return i;
        }
        ++i;
    }

    int key = table.size();
    table.push_back(literal(s));

    return key;
}

int LiteralPool::push_float(double fnum) {
    int i = 0;
    for(auto &t : table) {
        if(t.number == fnum) {
            return i;
        }

        ++i;
    }

    int key = table.size();
    table.push_back(literal(fnum));

    return key;
}

int LiteralPool::push_userfunc(userfunction func) {
    int key = table.size();
    table.push_back(literal(func));

    return key;
}
