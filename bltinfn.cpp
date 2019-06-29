#include "maxc.h"

extern NullObject Null;

MxcObject *println(MxcObject **stack, size_t narg) {
    error("internal error");

    return &Null;
}

MxcObject *println_int(MxcObject **stack, size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%d\n", a->inum32);
    DECREF(a);

    return &Null;
}

MxcObject *println_bool(MxcObject **stack, size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s\n", a->boolean ? "true" : "false");
    DECREF(a);

    return &Null;
}

MxcObject *println_char(MxcObject **stack, size_t narg) {
    ;//TODO

    return &Null;
}

MxcObject *println_string(MxcObject **stack, size_t narg) {
    debug("%p\n", stackptr);
    debug("%p\n", stackptr);

    StringObject *a = (StringObject *)Pop();
    printf("%s\n", a->str);

    DECREF(a);

    Pop();

    return &Null;
}

MxcObject *println_list(MxcObject **stack, size_t narg) {
    ;//TODO

    return &Null;
}

bltinfn_ty bltinfns[6] = {
    println,
    println_int,
    println_bool,
    println_char,
    println_string,
    println_list
};

