#include "function.h"
#include "error.h"
#include "object/object.h"
#include "vm.h"

extern NullObject Null;

userfunction *New_Userfunction(Bytecode *c, Varlist *v) {
    userfunction *u = malloc(sizeof(userfunction));

    u->code = c->code;
    u->codesize = c->len;
    u->nlvars = v->vars->len;

    return u;
}

MxcObject *print(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    while(narg --> 0) {
        StringObject *ob = (StringObject *)*--stackptr;

        printf("%s", ob->str);
    }

    Mxc_RetNull();
}

MxcObject *println(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    while(narg --> 0) {
        StringObject *ob = (StringObject *)*--stackptr;

        printf("%s", ob->str);
    }

    putchar('\n');

    Mxc_RetNull();
}

MxcObject *string_size(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    StringObject *ob = (StringObject *)*--stackptr;

    return (MxcObject *)new_intobject(strlen(ob->str));
}

MxcObject *string_isempty(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    StringObject *ob = (StringObject *)*--stackptr;
    if(strlen(ob->str) == 0)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

MxcObject *int_tofloat(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    IntObject *ob = (IntObject *)*--stackptr;
    return (MxcObject *)new_floatobject((double)ob->inum);
}

MxcObject *object_id(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    MxcObject *ob = *--stackptr;

    return (MxcObject *)new_intobject((size_t)ob);
}

MxcObject *mxcerror(MxcObject ***spp, size_t narg) {
    MxcObject **stackptr = *spp;

    StringObject *ob = (StringObject *)*--stackptr;
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
