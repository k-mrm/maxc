#include <stdlib.h>
#include <string.h>

#include "module.h"
#include "error/error.h"
#include "object/object.h"
#include "object/intobject.h"
#include "vm.h"
#include "mem.h"
#include "frame.h"
#include "gc.h"

Vector *Global_Cbltins;

MxcValue print_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    for(int i = narg - 1; i >= 0; --i) {
        MxcValue ob = sp[i];
        MxcString *strob = ostr(mval2str(ob));
        printf("%s", strob->str);
    }

    return mval_null;
}

MxcValue println_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    for(int i = narg - 1; i >= 0; --i) {
        MxcValue ob = sp[i];
        MxcString *strob = ostr(mval2str(ob));
        printf("%s", strob->str);
    }
    putchar('\n');

    Mxc_RetNull();
}

MxcValue strlen_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcString *ob = ostr(sp[0]);
    int len = ITERABLE(ob)->length;
    return mval_int(len);
}

MxcValue int_tofloat_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcValue val = sp[0];
    double fnum = (double)val.num;

    return mval_float(fnum);
}

MxcValue object_id_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcValue ob = sp[0];
    intptr_t id = (intptr_t)optr(ob);
    DECREF(ob);

    return mval_int(id);
}

MxcValue sys_exit_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcValue i = sp[0];
    exit(i.num);

    return mval_null;
}

MxcValue readline_core(Frame *f, MxcValue *sp, size_t narg) {
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

    if(rs.str) {
        return new_string(rs.str, strlen(rs.str));
    }
    else {
        return new_string_static("", 0);
    }
}

MxcValue list_len_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(narg);
    MxcList *ob = olist(sp[0]);

    return mval_int(ITERABLE(ob)->length);
}

MxcValue gc_run_core(Frame *f, MxcValue *sp, size_t narg) {
    INTERN_UNUSE(f);
    INTERN_UNUSE(sp);
    INTERN_UNUSE(narg);
    gc_run();

    return mval_null;
}

void builtin_Init() {
    Global_Cbltins = New_Vector();

    define_cmethod(Global_Cbltins, "print", print_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "println", println_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "echo", println_core, mxcty_none, mxcty_any_vararg, NULL);
    define_cmethod(Global_Cbltins, "len", strlen_core, mxcty_int, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "tofloat", int_tofloat_core, mxcty_float, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "objectid", object_id_core, mxcty_int, mxcty_any, NULL);
    define_cmethod(Global_Cbltins, "exit", sys_exit_core, mxcty_none, mxcty_int, NULL);
    define_cmethod(Global_Cbltins, "readline", readline_core, mxcty_string, NULL);
    define_cmethod(Global_Cbltins, "gc_run", gc_run_core, mxcty_none, NULL);
}

