#include "function.h"
#include "error.h"
#include "object.h"
#include "vm.h"

extern NullObject Null;

userfunction *New_Userfunction(Bytecode *c, Varlist *v) {
    userfunction *u = malloc(sizeof(userfunction));

    u->code = (uint8_t *)malloc(c->len * sizeof(uint8_t));
    memcpy(u->code, c->code, c->len * sizeof(uint8_t));
    u->codesize = c->len;

    u->nlvars = v->vars->len;

    return u;
}

MxcObject *print(size_t narg) {
    error("internal error: print called");

    Mxc_RetNull();
}

MxcObject *print_int(size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%ld", a->inum);
    DECREF(a);

    Mxc_RetNull();
}

MxcObject *print_float(size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf", f->fnum);
    DECREF(f);

    Mxc_RetNull();
}

MxcObject *print_bool(size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s", a->boolean ? "true" : "false");
    DECREF(a);

    Mxc_RetNull();
}

MxcObject *print_char(size_t narg) {
    ; // TODO

    Mxc_RetNull();
}

MxcObject *print_string(size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s", a->str);

    DECREF(a);

    Mxc_RetNull();
}

MxcObject *print_list(size_t narg) {
    ; // TODO
}

MxcObject *println(size_t narg) {
    error("internal error: println called");

    Mxc_RetNull();
}

MxcObject *println_int(size_t narg) {
    IntObject *a = (IntObject *)Pop();

    printf("%ld\n", a->inum);
    DECREF(a);

    Mxc_RetNull();
}

MxcObject *println_float(size_t narg) {
    FloatObject *f = (FloatObject *)Pop();

    printf("%lf\n", f->fnum);
    DECREF(f);

    Mxc_RetNull();
}

MxcObject *println_bool(size_t narg) {
    BoolObject *a = (BoolObject *)Pop();

    printf("%s\n", a->boolean ? "true" : "false");
    DECREF(a);

    Mxc_RetNull();
}

MxcObject *println_char(size_t narg) {
    ; // TODO

    Mxc_RetNull();
}

MxcObject *println_string(size_t narg) {
    StringObject *a = (StringObject *)Pop();
    printf("%s\n", a->str);

    DECREF(a);

    Mxc_RetNull();
}

MxcObject *println_list(size_t narg) {
    ; // TODO

    Mxc_RetNull();
}

MxcObject *string_size(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    return (MxcObject *)alloc_intobject(strlen(ob->str));
}

MxcObject *string_isempty(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    if(strlen(ob->str) == 0)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

MxcObject *int_tofloat(size_t narg) {
    IntObject *ob = (IntObject *)Pop();

    return (MxcObject *)alloc_floatobject((double)ob->inum);
}

MxcObject *object_id(size_t narg) {
    MxcObject *ob = Pop();

    return (MxcObject *)alloc_intobject((size_t)ob);
}

MxcObject *mxcerror(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    error_flag = 1;

    return (MxcObject *)new_errorobject(ob->str);
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
    string_size,
    string_isempty,
    int_tofloat,
    object_id,
    mxcerror,
};
