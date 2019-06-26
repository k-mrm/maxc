#include "maxc.h"

extern NullObject Null;

MxcObject *println(MxcObject **stack, size_t narg) {
    error("internal error");

    return &Null;
}

MxcObject *println_int(MxcObject **stack, size_t narg) {
    printf("%d\n", ((IntObject *)Pop())->inum32);

    return &Null;
}

MxcObject *println_bool(MxcObject **stack, size_t narg) {
    printf("%s\n", ((BoolObject *)Pop())->boolean ? "true" : "false");

    return &Null;
}

MxcObject *println_char(MxcObject **stack, size_t narg) {
    ;//TODO

    return &Null;
}

MxcObject *println_string(MxcObject **stack, size_t narg) {
    debug("%p\n", stackptr);
    debug("%p\n", stackptr);
    printf("%s\n", ((StringObject *)Pop())->str);

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

