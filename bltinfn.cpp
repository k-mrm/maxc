#include "bltinfn.h"
#include "error.h"
#include "vm.h"

extern NullObject Null;

MxcObject *print(MxcObject **stack, size_t narg) {
    error("internal error: print called");

    return &Null;
}

MxcObject *print_int(MxcObject **stack, size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%d", a->inum32);
    DECREF(a);

    return &Null;
}

MxcObject *print_float(MxcObject **stack, size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf", f->fnum);
    DECREF(f);

    return &Null;
}

MxcObject *print_bool(MxcObject **stack, size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s", a->boolean ? "true" : "false");
    DECREF(a);

    return &Null;
}

MxcObject *print_char(MxcObject **stack, size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *print_string(MxcObject **stack, size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s", a->str);

    DECREF(a);

    Pop();

    return &Null;
}

MxcObject *print_list(MxcObject **stack, size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *println(MxcObject **stack, size_t narg) {
    error("internal error: println called");

    return &Null;
}

MxcObject *println_int(MxcObject **stack, size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%d\n", a->inum32);
    DECREF(a);

    return &Null;
}

MxcObject *println_float(MxcObject **stack, size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf\n", f->fnum);
    DECREF(f);

    return &Null;
}

MxcObject *println_bool(MxcObject **stack, size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s\n", a->boolean ? "true" : "false");
    DECREF(a);

    return &Null;
}

MxcObject *println_char(MxcObject **stack, size_t narg) {
    ; // TODO

    return &Null;
}

MxcObject *println_string(MxcObject **stack, size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s\n", a->str);

    DECREF(a);

    Pop();

    return &Null;
}

MxcObject *println_list(MxcObject **stack, size_t narg) {
    ; // TODO

    return &Null;
}

bltinfn_ty bltinfns[14] = {print,
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
                           println_list};
