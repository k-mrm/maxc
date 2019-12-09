#include "function.h"
#include "error.h"
#include "object/object.h"
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
    while(narg-- > 0) {
        StringObject *ob = (StringObject *)Pop();

        printf("%s", ob->str);
    }

    Mxc_RetNull();
}

MxcObject *println(size_t narg) {
    while(narg-- > 0) {
        StringObject *ob = (StringObject *)Pop();

        printf("%s", ob->str);
    }

    putchar('\n');

    Mxc_RetNull();
}

MxcObject *string_size(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    return (MxcObject *)new_intobject(strlen(ob->str));
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

    return (MxcObject *)new_floatobject((double)ob->inum);
}

MxcObject *object_id(size_t narg) {
    MxcObject *ob = Pop();

    return (MxcObject *)new_intobject((size_t)ob);
}

MxcObject *mxcerror(size_t narg) {
    StringObject *ob = (StringObject *)Pop();

    error_flag++;

    return (MxcObject *)new_errorobject(ob->str);
}

bltinfn_ty bltinfns[] = {
    print,
    println,
    string_size,
    string_isempty,
    int_tofloat,
    object_id,
    mxcerror,
};
