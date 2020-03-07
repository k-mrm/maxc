/* implementation of char object */
#include <string.h>
#include <stdlib.h>

#include "object/charobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue new_char(char c) {
    MxcChar *ob = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    ob->ch = c;
    OBJIMPL(ob) = &char_objimpl; 

    return mval_obj(ob);
}

MxcValue new_char_ref(char *c) {
    MxcChar *ob = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    ob->ch = *c;
    OBJIMPL(ob) = &char_objimpl;

    return mval_obj(ob);
}

MxcValue char_copy(MxcObject *c) {
    MxcChar *n = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    memcpy(n, c, sizeof(MxcChar));
    n->ch = ((MxcChar *)c)->ch;

    return mval_obj(n);
}

void char_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

void char_guard(MxcObject *ob) {
    ob->gc_guard = 1;
}

void char_unguard(MxcObject *ob) {
    ob->gc_guard = 0;
}

void char_dealloc(MxcObject *self) {
    Mxc_free(self);
}

MxcValue char_tostring(MxcObject *self) {
    MxcChar *c = (MxcChar *)self;
    size_t len = 2;
    char *s = malloc(sizeof(char) * len);
    s[0] = c->ch;
    s[1] = '\0';

    return new_string(s, len);
}

MxcObjImpl char_objimpl = {
    "char",
    char_tostring,
    char_dealloc,
    char_copy,
    char_gc_mark,
    char_guard,
    char_unguard,
    0,
    0,
};
