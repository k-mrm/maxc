#include <stdlib.h>

#include "module.h"
#include "error/error.h"
#include "object/object.h"
#include "object/intobject.h"
#include "vm.h"
#include "mem.h"
#include "frame.h"
#include "gc.h"

Vector *Global_Cbltins;

MxcObject *print_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
    }

    Mxc_RetNull();
}

MxcObject *println_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    for(int i = narg - 1; i >= 0; --i) {
        MxcObject *ob = sp[i];
        StringObject *strob = OBJIMPL(ob)->tostring(ob);
        printf("%s", strob->str);
    }
    putchar('\n');

    Mxc_RetNull();
}

MxcObject *strlen_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    StringObject *ob = (StringObject *)sp[0];
    int len = ITERABLE(ob)->length;
    DECREF(ob);

    return (MxcObject *)new_intobject(len);
}

MxcObject *int_tofloat_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    IntObject *ob = (IntObject *)sp[0];
    double fnum = (double)ob->inum;
    DECREF(ob);

    return (MxcObject *)new_floatobject(fnum);
}

MxcObject *object_id_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcObject *ob = sp[0];
    size_t id = (size_t)ob;
    DECREF(ob);

    return (MxcObject *)new_intobject(id);
}

MxcObject *error_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    StringObject *ob = (StringObject *)sp[0];
    char *str = ob->str;
    error_flag++;

    return (MxcObject *)new_errorobject(str);
}

MxcObject *sys_exit_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    IntObject *i = (IntObject *)sp[0];
    exit(i->inum);

    DECREF(i);

    Mxc_RetNull();
}

MxcObject *readline_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
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
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    ListObject *ob = (ListObject *)sp[0];
    DECREF(ob);

    return (MxcObject *)new_intobject(ITERABLE(ob)->length);
}

MxcObject *gc_run_core(Frame *f, MxcObject **sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(sp);
    INTERN_UNUSE(narg);
    size_t before = heap_length();
    gc_run();
    size_t after = heap_length();
    printf("before: %zdbyte after: %zdbyte\n", before, after);

    Mxc_RetNull();
}

void builtin_Init() {
    Global_Cbltins = New_Vector();

    define_cmethod(Global_Cbltins, "print", print_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "println", println_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "len", strlen_core, mxcty_int, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "tofloat", int_tofloat_core, mxcty_float, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "objectid", object_id_core, mxcty_int, mxcty_any, NULL);
    define_cmethod(Global_Cbltins, "exit", sys_exit_core, mxcty_none, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "readline", readline_core, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "gc_run", gc_run_core, mxcty_none, NULL);
}

