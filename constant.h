#ifndef MAXC_CONST_H
#define MAXC_CONST_H

#include "bltinfn.h"
#include "maxc.h"

class NodeVariable;

struct const_t {
    const char *str; // str
    userfunction func;

    const_t(const char *s) : str(s) {}
    const_t(userfunction u) : str(nullptr), func(u) {}
};

class Constant {
  public:
    std::vector<const_t> table;

    int push_str(const char *);
    int push_userfunc(userfunction &);
};

#endif
