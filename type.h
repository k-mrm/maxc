#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include "maxc.h"

enum class CTYPE {
    NONE,
    INT,
    UINT,
    INT64,
    UINT64,
    DOUBLE,
    BOOL,
    CHAR,
    STRING,
    LIST,
    TUPLE,
    FUNCTION,
    UNINFERRED,
};

class Type;

typedef std::vector<Type *> Type_v;

struct type_t {
    CTYPE type;
    int size; // array size

    type_t() {}
    type_t(CTYPE ty) : type(ty) {}
    type_t(CTYPE ty, int size) : type(ty), size(size) {}
};

class Type {
  public:
    type_t type;
    Type *ptr = nullptr; // list
    Type_v tuple;
    Type_v fnarg;          // function arg
    Type *fnret = nullptr; // function rettype

    Type() {}
    Type(CTYPE ty) : type(ty) {}
    Type(CTYPE ty, int size) : type(ty, size) {} //?
    Type(Type *p) : type(CTYPE::LIST), ptr(p) {} // list
    Type(Type_v a, Type *r) :
        type(CTYPE::FUNCTION), fnarg(a), fnret(r) {} // function

    std::string show();
    int get_size();
    type_t &get();
    bool uninfer();
    bool isint();
    bool islist();
    bool isfloat();
    bool isstring();
    bool istuple();
    bool isobject();
    bool isfunction();
    void tupletype_push(Type *);
};

#endif
