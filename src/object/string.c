/* implementation of string object */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "object/strobject.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

StringObject *new_stringobject(char *s, bool isdyn) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ob->str = s;
    ob->isdyn = isdyn;
    ITERABLE(ob)->length = strlen(s);
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

MxcObject *string_copy(MxcObject *s) {
    StringObject *n = (StringObject *)Mxc_malloc(sizeof(StringObject));
    StringObject *old = (StringObject *)s;
    *n = *old; 

    char *olds = n->str;
    n->str = malloc(sizeof(char) * (ITERABLE(n)->length + 1));
    strcpy(n->str, olds);
    n->isdyn = true;

    return (MxcObject *)n;
}

void string_dealloc(MxcObject *s) {
    StringObject *str = (StringObject *)s;
    if(str->isdyn) {
        free(str->str);
    }
    Mxc_free(s);
}

MxcObject *str_index(MxcIterable *self, size_t idx) {
    StringObject *str = (StringObject *)self;
    if(self->length <= idx) return NULL;

    return (MxcObject *)new_charobject_ref(&str->str[idx]);
}

MxcObject *str_index_set(MxcIterable *self, size_t idx, MxcObject *a) {
    StringObject *str = (StringObject *)self;
    if(self->length <= idx) return NULL;
    str->str[idx] = ((CharObject *)a)->ch;

    return a;
}

StringObject *str_concat(StringObject *a, StringObject *b) {
    int len = ITERABLE(a)->length + ITERABLE(b)->length;
    char *res = malloc(sizeof(char) * len + 1);
    strcpy(res, a->str);
    strcat(res, b->str);

    DECREF(a);
    DECREF(b);

    return new_stringobject(res, true);
}

StringObject *string_tostring(MxcObject *ob) {
    return (StringObject *)OBJIMPL(ob)->copy(ob);
}

MxcObjImpl string_objimpl = {
    "string",
    string_tostring,
    string_dealloc,
    string_copy,
    0,
    str_index,
    str_index_set,
};
