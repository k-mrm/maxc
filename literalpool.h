#ifndef MAXC_CONST_H
#define MAXC_CONST_H

#include "bltinfn.h"
#include "maxc.h"

struct literal {
    std::string str; // str
    double number;
    userfunction func;

    literal(std::string s) : str(s) {}
    literal(double fnum) : number(fnum) {}
    literal(userfunction u) : func(u) {}
};

class LiteralPool {
  public:
    std::vector<literal> table;

    int push_str(std::string &);
    int push_float(double);
    int push_userfunc(userfunction);
};

#endif
