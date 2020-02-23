/* implementation of char object */
#include <string.h>
#include <stdlib.h>

#include "object/charobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcChar *new_char(char c) {
    MxcChar *ob = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    ob->ch = c;
    OBJIMPL(ob) = &char_objimpl; 

    return ob;
}

MxcChar *new_char_ref(char *c) {
    MxcChar *ob = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    ob->ch = *c;
    OBJIMPL(ob) = &char_objimpl;

    return ob;
}

MxcObject *char_copy(MxcObject *c) {
    MxcChar *n = (MxcChar *)Mxc_malloc(sizeof(MxcChar));
    memcpy(n, c, sizeof(MxcChar));
    n->ch = ((MxcChar *)c)->ch;

    return (MxcObject *)n;
}

void char_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

void char_dealloc(MxcObject *self) {
    Mxc_free(self);
}

MxcString *char_tostring(MxcObject *self) {
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
    0,
    0,
};
