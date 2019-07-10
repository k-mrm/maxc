#ifndef MAXC_CONST_H
#define MAXC_CONST_H

struct const_t {
    const char *str; // str
    NodeVariable *var;
    userfunction func;

    const_t(const char *s) : str(s), var(nullptr) {}
    const_t(NodeVariable *v) : str(nullptr), var(v) {}
    const_t(userfunction u) : str(nullptr), var(nullptr), func(u) {}
};

class Constant {
  public:
    std::vector<const_t> table;

    int push_var(NodeVariable *);
    int push_str(const char *);
    int push_userfunc(userfunction &);
};

#endif
