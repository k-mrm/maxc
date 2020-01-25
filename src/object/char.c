/* implementation of char object */
#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

CharObject *new_charobject(char c) {
    CharObject *ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = c;
    OBJIMPL(ob) = &char_objimpl; 

    return ob;
}

CharObject *new_charobject_ref(char *c) {
    CharObject *ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = *c;
    OBJIMPL(ob) = &char_objimpl;

    return ob;
}

void char_dealloc(MxcObject *self) {
    Mxc_free(self);
}

StringObject *char_tostring(MxcObject *self) {
    CharObject *c = (CharObject *)self;
    char *s = malloc(sizeof(char) * 2);
    s[0] = c->ch;
    s[1] = '\0';

    return new_stringobject(s);
}

MxcObjImpl char_objimpl = {
    "char",
    char_tostring,
    char_dealloc,
    sizeof(CharObject),
    0,
};
