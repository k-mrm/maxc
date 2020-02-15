#include <stdlib.h>

#include "module.h"
#include "error/error.h"
#include "object/object.h"
#include "object/intobject.h"
#include "vm.h"
#include "mem.h"
#include "frame.h"

Vector *Global_Cbltins;

MxcObject *print_core(Frame *f, MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
        OBJIMPL(strob)->dealloc((MxcObject *)strob);
    }

    Mxc_RetNull();
}

MxcObject *println_core(Frame *f, MxcObject **sp, size_t narg) {
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
        OBJIMPL(strob)->dealloc((MxcObject *)strob);
    }
    putchar('\n');

    Mxc_RetNull();
}

MxcObject *strlen_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    StringObject *ob = (StringObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject(ITERABLE(ob)->length);
}

MxcObject *int_tofloat_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    IntObject *ob = (IntObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_floatobject((double)ob->inum);
}

MxcObject *object_id_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    MxcObject *ob = sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject((size_t)ob);
}

MxcObject *error_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    StringObject *ob = (StringObject *)sp[0];
    error_flag++;

    return (MxcObject *)new_errorobject(ob->str);
}

MxcObject *sys_exit_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    IntObject *i = (IntObject *)sp[0];
    exit(i->inum);

    DECREF(i);

    Mxc_RetNull();
}

MxcObject *readline_core(Frame *f, MxcObject **sp, size_t narg) {
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

MxcObject *list_len_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(narg);
    ListObject *ob = (ListObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject(ITERABLE(ob)->length);
}

void builtin_Init() {
    Type *errty = New_Type(CTYPE_ERROR);
    Global_Cbltins = New_Vector();

    define_cmethod(Global_Cbltins, "print", print_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "println", println_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "len", strlen_core, mxcty_int, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "tofloat", int_tofloat_core, mxcty_float, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "objectid", object_id_core, mxcty_int, mxcty_any, NULL);
    define_cmethod(Global_Cbltins, "error", sys_exit_core,  errty, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "exit", sys_exit_core, mxcty_none, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "readline", readline_core, mxcty_string, NULL);
}

