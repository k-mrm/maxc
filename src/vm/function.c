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

MxcObject *print(MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        printf("%s", OBJIMPL(ob)->tostring(ob)->str);
    }

    Mxc_RetNull();
}

MxcObject *println(MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        printf("%s", OBJIMPL(ob)->tostring(ob)->str);
    }
    putchar('\n');

    Mxc_RetNull();
}

MxcObject *string_size(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    StringObject *ob = (StringObject *)sp[0];

    return (MxcObject *)new_intobject(ob->len);
}

MxcObject *string_isempty(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    StringObject *ob = (StringObject *)sp[0];
    if(ob->len == 0)
        Mxc_RetTrue();
    else
        Mxc_RetFalse();
}

MxcObject *int_tofloat(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    IntObject *ob = (IntObject *)sp[0];

    return (MxcObject *)new_floatobject((double)ob->inum);
}

MxcObject *object_id(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    MxcObject *ob = sp[0];

    return (MxcObject *)new_intobject((size_t)ob);
}

MxcObject *mxcerror(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    StringObject *ob = (StringObject *)sp[0];
    error_flag++;

    return (MxcObject *)new_errorobject(ob->str);
}

MxcObject *mxcsys_exit(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);

    IntObject *i = (IntObject *)sp[0];
    exit(i->inum);

    Mxc_RetNull();
}

MxcObject *mxc_readline(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(sp);
    INTERN_UNUSE(narg);


    Mxc_RetNull();
}

bltinfn_ty bltinfns[] = {
    print,              /* BLTINFN_PRINT */
    println,            /* BLTINFN_PRINTLN */
    string_size,        /* BLTINFN_STRINGSIZE */
    string_isempty,     /* BLTINFN_STRINGISEMPTY */
    int_tofloat,        /* BLTINFN_INTTOFLOAT */
    object_id,          /* BLTINFN_OBJECTID */ 
    mxcerror,           /* BLTINFN_ERROR */ 
    mxcsys_exit,        /* BLTINFN_EXIT */
    mxc_readline,       /* BLTINFN_READLINE */
};
