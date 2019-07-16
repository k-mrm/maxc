#ifndef MAXC_CONST_H
#define MAXC_CONST_H

#include "bltinfn.h"
#include "maxc.h"

class NodeVariable;

struct const_t {
    std::string str; // str
    double number;
    userfunction func;

    const_t(std::string s) : str(s) {}
    const_t(double fnum) : number(fnum) {}
    const_t(userfunction u) : func(u) {}
};

class Constant {
  public:
    std::vector<const_t> table;

    int push_str(std::string &);
    int push_float(double);
    int push_userfunc(userfunction &);
};

#endif
