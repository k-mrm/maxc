#include "maxc.h"

#define Pop() (*(--stack))

MxcObject *println(MxcObject **stack, size_t narg) {
    error("internal error");
}

MxcObject *println_int(MxcObject **stack, size_t narg) {
    printf("%d\n", ((IntObject *)Pop())->inum32);
}

MxcObject *println_bool(MxcObject **stack, size_t narg) {
    printf("%s\n", ((BoolObject *)Pop())->boolean ? "true" : "false");
}

MxcObject *println_char(MxcObject **stack, size_t narg) {
    ;//TODO
}

MxcObject *println_string(MxcObject **stack, size_t narg) {
    printf("%s\n", ((StringObject *)Pop())->str);
}

MxcObject *println_list(MxcObject **stack, size_t narg) {
    ;//TODO
}

bltinfn_ty bltinfns[6] = {
    println,
    println_int,
    println_bool,
    println_char,
    println_string,
    println_list
};
