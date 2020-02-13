#include <stdlib.h>

#include "builtins.h"
#include "error/error.h"
#include "object/object.h"
#include "vm.h"
#include "mem.h"

Varlist *bltin_funcs;

MxcObject *print(MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
        OBJIMPL(strob)->dealloc((MxcObject *)strob);
    }

    Mxc_RetNull();
}

MxcObject *println(MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
        OBJIMPL(strob)->dealloc((MxcObject *)strob);
    }
    putchar('\n');

    Mxc_RetNull();
}

MxcObject *string_size(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    StringObject *ob = (StringObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject(ITERABLE(ob)->length);
}

MxcObject *int_tofloat(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    IntObject *ob = (IntObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_floatobject((double)ob->inum);
}

MxcObject *object_id(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    MxcObject *ob = sp[0];
    DECREF(ob);

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

    DECREF(i);

    Mxc_RetNull();
}

MxcObject *mxc_readline(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(sp);
    INTERN_UNUSE(narg);
    size_t cur;
    ReadStatus rs = intern_readline(1024, &cur, "", 0); 
    if(rs.err.eof) {
        // TODO
    }
    if(rs.err.toolong) {
        // TODO
    }

    return (MxcObject *)new_stringobject(rs.str ? rs.str : "",
                                         rs.str ? true : false);
}

MxcObject *list_len(MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    ListObject *ob = (ListObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject(ITERABLE(ob)->length);
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
    list_len,           /* BLTINFN_LISTLEN */
};
