#include "bltinfn.h"
#include "error.h"
#include "vm.h"
#include "object.h"

extern NullObject Null;

void new_userfunction(userfunction &u, bytecode c, Varlist v) {
    u.code = (uint8_t *)malloc(c.size() * sizeof(uint8_t));
    memcpy(u.code, &c[0], c.size() * sizeof(uint8_t));
    u.codelength = c.size();

    u.nlvars = v.var_v.size();
}

MxcObject *print(size_t narg) {
    error("internal error: print called");

    return &Null;
}

MxcObject *print_int(size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%d", a->inum32);
    DECREF(a);

    return &Null;
}

MxcObject *print_float(size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf", f->fnum);
    DECREF(f);

    return &Null;
}

MxcObject *print_bool(size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s", a->boolean ? "true" : "false");
    DECREF(a);

    return &Null;
}

MxcObject *print_char(size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *print_string(size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s", a->str);

    DECREF(a);

    return &Null;
}

MxcObject *print_list(size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *println(size_t narg) {
    error("internal error: println called");

    return &Null;
}

MxcObject *println_int(size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%d\n", a->inum32);
    DECREF(a);

    return &Null;
}

MxcObject *println_float(size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf\n", f->fnum);
    DECREF(f);

    return &Null;
}

MxcObject *println_bool(size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s\n", a->boolean ? "true" : "false");
    DECREF(a);

    return &Null;
}

MxcObject *println_char(size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *println_string(size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s\n", a->str);

    DECREF(a);

    return &Null;
}

MxcObject *println_list(size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *string_size(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    return Object::alloc_intobject(strlen(ob->str));
}

bltinfn_ty bltinfns[] = {
    print,
    print_int,
    print_float,
    print_bool,
    print_char,
    print_string,
    print_list,
    println,
    println_int,
    println_float,
    println_bool,
    println_char,
    println_string,
    println_list,
    string_size
};
